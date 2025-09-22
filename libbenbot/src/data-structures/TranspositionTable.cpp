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

// the transposition table is a bucket-like hash map
// each "cluster" is an array of entries; the root table is an array of clusters
// a coarse lookup using the Zobrist key gets you to the right cluster,
// then the lowest 16 bits of the Zobrist key are used to identify entries within a cluster

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint> // IWYU pragma: keep
#include <iterator>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/search/Bounds.hpp>
#include <libchess/util/Math.hpp>
#include <libchess/util/Memory.hpp>
#include <memory>
#include <new>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

namespace ben_bot {

using std::size_t;

static constexpr auto GENERATION_DELTA = 8uz;
static constexpr auto GENERATION_CYCLE = 255uz + GENERATION_DELTA;

struct TranspositionTable::Entry final {
    std::uint16_t key { 0 }; // the lowest 16 bits of the position's Zobrist key

    std::uint8_t depth { 0 };

    std::uint8_t generation { 0 };

    std::int16_t eval { 0 };

    EvalType evalType { EvalType::Alpha };

    Move move;

    [[nodiscard]] bool occupied() const noexcept
    {
        return std::cmp_greater(depth, 0);
    }

    [[nodiscard]] std::uint8_t relative_age(const std::uint8_t gen) const noexcept
    {
        static constexpr auto GENERATION_MASK = 248uz; // highest 5 bits

        return (GENERATION_CYCLE + gen - generation) & GENERATION_MASK;
    }

    [[nodiscard]] TTData read() const noexcept
    {
        std::optional<Move> bestMove;

        if (not move.is_null())
            bestMove.emplace(move);

        return {
            .searchedDepth = depth,
            .eval          = static_cast<int>(eval),
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

        if (data.bestMove.has_value())
            move = data.bestMove.value();
        else
            move = Move {};
    }
};

static constexpr auto ClusterSize = 5uz;

struct TranspositionTable::Cluster final {
    std::array<Entry, ClusterSize> records {};

    std::array<std::byte, 4uz> padding {}; // pad to 64 bytes
};

TranspositionTable::TranspositionTable(TranspositionTable&& other) noexcept
    : table { std::exchange(other.table, nullptr) }
    , clusterCount { std::exchange(other.clusterCount, 0uz) }
    , generation { std::exchange(other.generation, 0) }
{
}

TranspositionTable& TranspositionTable::operator=(TranspositionTable&& other) noexcept
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
    deallocate();

    clusterCount = sizeMB * 1024uz * 1024uz / sizeof(Cluster);

    table = static_cast<Cluster*>(chess::util::page_aligned_alloc(clusterCount * sizeof(Cluster)));

    if (table == nullptr)
        throw std::bad_alloc {};

    std::uninitialized_default_construct_n(table, clusterCount);
}

void TranspositionTable::clear()
{
    for (auto i = 0uz; i < clusterCount; ++i)
        std::ranges::fill(index_table(i).records, Entry {});
}

void TranspositionTable::deallocate()
{
    std::destroy_n(table, clusterCount);

    chess::util::page_aligned_free(table);
}

size_t TranspositionTable::hashfull() const
{
    size_t count { 0uz };

    for (auto i = 0uz; i < std::min(1000uz, clusterCount); ++i) {
        count += static_cast<size_t>(
            std::ranges::count_if(
                index_table(i).records,
                [gen = generation](const Entry& entry) {
                    return entry.occupied()
                       and entry.relative_age(gen) == 0uz;
                }));
    }

    return count / ClusterSize;
}

void TranspositionTable::new_search() noexcept
{
    generation += GENERATION_DELTA;
}

auto TranspositionTable::find_cluster(const Position::Hash key) const noexcept -> std::span<Entry>
{
    return index_table(
        chess::util::mul_hi64(key, clusterCount))
        .records;
}

std::optional<TTData> TranspositionTable::find(const Position& pos) const
{
    // the lowest 16 bits are the key within the cluster
    const auto key = static_cast<std::uint16_t>(pos.hash);

    const auto cluster = find_cluster(pos.hash);

    if (const auto it = std::ranges::find_if(
            cluster,
            [key](const Entry& entry) { return entry.key == key; });
        it != cluster.end() and it->occupied()) {
        return it->read();
    }

    return std::nullopt;
}

auto TranspositionTable::probe_eval(
    const Position& pos, const size_t depth,
    const search::Bounds& bounds) const
    -> std::optional<ProbedEval>
{
    if (const auto record = find(pos);
        record.has_value() and record->searchedDepth >= depth) {
        switch (record->evalType) {
            using enum EvalType;

            case Exact:
                return std::make_pair(record->eval, record->evalType);

            case Alpha: {
                if (record->eval <= bounds.alpha)
                    return std::make_pair(bounds.alpha, record->evalType);

                break;
            }

            case Beta: {
                if (record->eval >= bounds.beta)
                    return std::make_pair(bounds.beta, record->evalType);

                break;
            }

            default: std::unreachable();
        }
    }

    return std::nullopt;
}

std::optional<Move> TranspositionTable::get_best_response(
    const Position& pos, const Move& move) const
{
    if (const auto record = find(after_move(pos, move)))
        return record->bestMove;

    return std::nullopt;
}

void TranspositionTable::store(const Position& pos, const TTData& record)
{
    const auto key = static_cast<std::uint16_t>(pos.hash);

    auto cluster = find_cluster(pos.hash);

    for (auto& stored : cluster) {
        if (stored.key == key) {
            // this position was already stored in the table
            // keep the old evaluation if it was an exact one & the new one isn't,
            // or if the new evaluation is a lower depth than the old one

            const bool shouldReplace
                = record.searchedDepth > stored.depth
               or (stored.evalType != EvalType::Exact
                   and record.evalType == EvalType::Exact);

            if (shouldReplace)
                stored.save(key, record, generation);

            return;
        }

        if (not stored.occupied()) {
            // empty slot
            stored.save(key, record, generation);
            return;
        }
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
    chess::util::prefetch(
        find_cluster(pos.hash).data());
}

} // namespace ben_bot
