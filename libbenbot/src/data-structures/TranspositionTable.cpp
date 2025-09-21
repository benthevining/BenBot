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

#include <algorithm>
#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/search/Bounds.hpp>
#include <libchess/util/Math.hpp>
#include <libchess/util/Memory.hpp>
#include <memory>
#include <new>
#include <optional>
#include <utility>

namespace ben_bot {

using std::size_t;

static constexpr auto ClusterSize = 3uz;

struct TranspositionTable::Cluster final {
    std::array<Record, ClusterSize> records;
};

TranspositionTable::TranspositionTable()
{
    resize(100uz);
}

using chess::util::page_aligned_free;

TranspositionTable::~TranspositionTable()
{
    std::destroy_n(table, clusterCount);
    page_aligned_free(table);
}

void TranspositionTable::resize(const size_t sizeMB)
{
    // deallocate previous table
    std::destroy_n(table, clusterCount);
    page_aligned_free(table);

    clusterCount = sizeMB * 1024uz * 1024uz / sizeof(Cluster);

    table = static_cast<Cluster*>(chess::util::page_aligned_alloc(clusterCount * sizeof(Cluster)));

    if (table == nullptr)
        throw std::bad_alloc {};

    std::uninitialized_default_construct_n(table, clusterCount);
}

void TranspositionTable::clear()
{
    for (auto i = 0uz; i < clusterCount; ++i)
        std::ranges::fill(table[i].records, Record {});
}

size_t TranspositionTable::hashfull() const
{
    // TODO
}

void TranspositionTable::new_search() noexcept
{
    // TODO
}

auto TranspositionTable::first_entry(const Position::Hash key) const noexcept -> Record*
{
    const auto idx = chess::util::mul_hi64(key, clusterCount);

    return table[idx].records.data();
}

auto TranspositionTable::find(const Position& pos) const -> const Record*
{
    const auto* first = first_entry(pos.hash);

    for (auto i = 0uz; i < ClusterSize; ++i)
        if (first[i].hash == pos.hash)
            return &first[i];

    return nullptr;
}

auto TranspositionTable::probe_eval(
    const Position& pos, const size_t depth,
    const search::Bounds& bounds) const
    -> std::optional<ProbedEval>
{
    if (const auto* record = find(pos);
        record != nullptr and record->searchedDepth >= depth) {
        switch (record->evalType) {
            using enum Record::EvalType;

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

void TranspositionTable::store(const Position& pos, Record record)
{
    record.hash = pos.hash;

    auto* first = first_entry(pos.hash);

    for (auto i = 0uz; i < ClusterSize; ++i) {
        if (first[i].hash == pos.hash) {
            // this position was already stored in the table
            // keep the old evaluation if it was an exact one & the new one isn't,
            // or if the new evaluation is a lower depth than the old one

            auto& stored = first[i];

            const bool shouldReplace
                = record.searchedDepth > stored.searchedDepth
               or (stored.evalType != Record::EvalType::Exact
                   and record.evalType == Record::EvalType::Exact);

            if (shouldReplace)
                stored = record;

            return;
        }

        if (first[i].searchedDepth == 0uz) {
            // empty slot
            first[i] = record;
            return;
        }
    }

    // TODO: implement replacement strategy
}

} // namespace ben_bot
