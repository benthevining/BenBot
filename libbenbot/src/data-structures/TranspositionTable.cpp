/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

// the transposition table is a bucket-style hash map
// each "cluster" is an array of entries; the root table is an array of clusters
// a coarse lookup using the Zobrist key gets you to the right cluster,
// then the lowest 16 bits of the Zobrist key are used to identify entries within a cluster

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint> // IWYU pragma: keep
#include <functional>
#include <iterator>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Bounds.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/util/Math.hpp>
#include <libchess/util/Memory.hpp>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

namespace ben_bot {

using std::size_t;

inline constexpr auto GENERATION_DELTA = 8uz;
inline constexpr auto GENERATION_CYCLE = 255uz + GENERATION_DELTA;

struct TranspositionTable::Entry final {
    std::uint16_t key { UINT16_C(0) }; // the lowest 16 bits of the position's Zobrist key

    std::uint8_t depth { UINT8_C(0) }; // empty slots are marked with a depth of 0

    std::uint8_t generation { UINT8_C(0) };

    eval::Value eval { UINT16_C(0) };

    EvalType evalType { EvalType::Alpha };

    Move move;

    [[nodiscard]] auto occupied() const noexcept -> bool
    {
        return std::cmp_greater(depth, 0);
    }

    [[nodiscard]] auto relative_age(const std::uint8_t gen) const noexcept -> std::uint8_t
    {
        static constexpr auto GENERATION_MASK = 248uz; // highest 5 bits

        return (GENERATION_CYCLE + gen - generation) & GENERATION_MASK;
    }

    [[nodiscard]] auto read() const noexcept -> TTData
    {
        std::optional<Move> bestMove;

        if (not move.is_null())
            bestMove.emplace(move);

        return {
            .searchedDepth = depth,
            .eval          = eval,
            .evalType      = evalType,
            .bestMove      = bestMove
        };
    }

    void save(
        const std::uint16_t keyToUse,
        const TTData&       data,
        const std::uint8_t  gen) noexcept
    {
        key        = keyToUse;
        depth      = static_cast<std::uint_least8_t>(data.searchedDepth);
        generation = gen;
        eval       = static_cast<std::int16_t>(data.eval);
        evalType   = data.evalType;
        move       = data.bestMove.value_or(Move { });
    }
};

#if defined(__cpp_lib_hardware_interference_size) and (__cpp_lib_hardware_interference_size >= 201703L)
inline constexpr auto CACHE_LINE_SIZE = std::hardware_constructive_interference_size;
#else
#    warning std::hardware_constructive_interference_size not available, using default value
inline constexpr auto CACHE_LINE_SIZE = 32uz;
#endif

struct alignas(CACHE_LINE_SIZE) TranspositionTable::Cluster final {
    static constexpr auto NumRecords = CACHE_LINE_SIZE / sizeof(Entry);

    std::array<Entry, NumRecords> records { };
};

TranspositionTable::TranspositionTable(TranspositionTable&& other) noexcept
    : table { std::exchange(other.table, nullptr) }
    , clusterCount { std::exchange(other.clusterCount, 0uz) }
    , generation { std::exchange(other.generation, 0) }
{
    // NB. need to do these asserts within class scope so Cluster is available

    static_assert(
        std::has_single_bit(sizeof(Cluster)),
        "TranspositionTable::Cluster size should be a power of 2!");

    static_assert(
        sizeof(Cluster) <= CACHE_LINE_SIZE,
        "TranspositionTable::Cluster should fit in a CPU cache line!");
}

auto TranspositionTable::operator=(TranspositionTable&& other) noexcept -> TranspositionTable&
{
    deallocate();

    table        = std::exchange(other.table, nullptr);
    clusterCount = std::exchange(other.clusterCount, 0uz);
    generation   = std::exchange(other.generation, 0);

    return *this;
}

// this function serves to add a bounds check and avoid warnings about raw pointer arithmetic
auto TranspositionTable::index_table(const size_t clusterIdx) const noexcept -> Cluster&
{
    assert(clusterIdx < clusterCount);

    return *std::next(
        table,
        static_cast<std::ptrdiff_t>(clusterIdx));
}

void TranspositionTable::resize(const size_t sizeMB)
{
    const auto newClusterCount = (sizeMB * 1024uz * 1024uz) / sizeof(Cluster);

    if (clusterCount == newClusterCount)
        return;

    deallocate();

    clusterCount = newClusterCount;

    table = static_cast<Cluster*>(
        chess::util::memory::page_aligned_alloc(
            clusterCount * sizeof(Cluster)));

    if (table == nullptr) {
        clusterCount = 0uz;
        throw std::bad_alloc { };
    }

    std::uninitialized_default_construct_n(table, clusterCount);
}

void TranspositionTable::clear()
{
    for (auto i = 0uz; i < clusterCount; ++i)
        std::ranges::fill(index_table(i).records, Entry { });
}

void TranspositionTable::deallocate()
{
    std::destroy_n(table, clusterCount);

    chess::util::memory::page_aligned_free(table);

    table        = nullptr;
    clusterCount = 0uz;
}

auto TranspositionTable::hashfull() const -> size_t
{
    const auto indices = std::views::iota(
        0uz,
        std::min(1000uz, clusterCount));

    const auto count = std::transform_reduce(
        indices.begin(), indices.end(),
        0uz,
        std::plus<void> { },
        [this](const size_t idx) {
            return static_cast<size_t>(std::ranges::count_if(
                index_table(idx).records,
                [gen = generation](const Entry& entry) {
                    return entry.occupied()
                       and entry.relative_age(gen) == 0uz;
                }));
        });

    return count / Cluster::NumRecords;
}

void TranspositionTable::new_search() noexcept
{
    generation += GENERATION_DELTA;
}

auto TranspositionTable::find_cluster(const Position::Hash key) const noexcept -> std::span<Entry>
{
    return index_table(
        chess::util::math::mul_hi64(key, clusterCount))
        .records;
}

auto TranspositionTable::find(const Position& pos) const -> std::optional<TTData>
{
    const auto cluster = find_cluster(pos.hash);

    if (const auto it = std::ranges::find_if(
            cluster,
            // the lowest 16 bits are the key within the cluster
            [key = static_cast<std::uint16_t>(pos.hash)](const Entry& entry) {
                return entry.occupied() and entry.key == key;
            });
        it != cluster.end()) {
        return it->read();
    }

    return std::nullopt;
}

auto TranspositionTable::probe_eval(
    const Position& pos, const size_t depth,
    const search::Bounds& bounds) const
    -> std::optional<ProbedEval>
{
    return find(pos)
        .and_then([depth](const TTData& data) -> std::optional<TTData> {
            if (data.searchedDepth < depth)
                return std::nullopt;

            return data;
        })
        .and_then([bounds](const TTData& data) -> std::optional<ProbedEval> {
            switch (data.evalType) {
                case EvalType::Exact:
                    return std::make_pair(data.eval, data.evalType);

                case EvalType::Alpha: {
                    if (data.eval < bounds.alpha)
                        return std::make_pair(bounds.alpha, data.evalType);

                    break;
                }

                case EvalType::Beta: {
                    if (data.eval >= bounds.beta)
                        return std::make_pair(bounds.beta, data.evalType);

                    break;
                }
            }

            return std::nullopt;
        });
}

auto TranspositionTable::get_best_response(
    const Position& pos, const Move move) const -> std::optional<Move>
{
    return find(after_move(pos, move))
        .and_then(&TTData::bestMove);
}

void TranspositionTable::store(const Position& pos, const TTData& record)
{
    const auto key = static_cast<std::uint16_t>(pos.hash);

    const auto cluster = find_cluster(pos.hash);

    // check if this position is already present in this cluster
    if (const auto it = std::ranges::find_if(cluster,
            [key](const Entry& entry) { return entry.occupied() and entry.key == key; });
        it != cluster.end()) {
        // keep the old evaluation if it was an exact one & the new one isn't,
        // or if the new evaluation is a lower depth than the old one
        const bool shouldReplace
            = record.searchedDepth > it->depth
           or (it->evalType != EvalType::Exact
               and record.evalType == EvalType::Exact);

        if (shouldReplace)
            it->save(key, record, generation);

        return;
    }

    // check for empty slot
    if (const auto it = std::ranges::find_if(cluster,
            [](const Entry& entry) { return not entry.occupied(); });
        it != cluster.end()) {
        it->save(key, record, generation);
        return;
    }

    // choose one of the entries in this cluster to overwrite
    // an entry is considered more valuable if its replacement score is higher
    // (ie, we want to replace the entry with the lowest score)
    auto replacement_score = [gen = generation](const Entry& entry) {
        return entry.depth - entry.relative_age(gen);
    };

    auto* replace = cluster.data();

    for (auto& entry : cluster | std::views::drop(1uz))
        if (replacement_score(*replace) > replacement_score(entry))
            replace = &entry;

    replace->save(key, record, generation);
}

void TranspositionTable::prefetch(const Position& pos) const noexcept
{
    chess::util::memory::prefetch(
        find_cluster(pos.hash).data());
}

} // namespace ben_bot
