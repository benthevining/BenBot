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

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <optional>
#include <utility>

namespace ben_bot {

using std::size_t;

auto TranspositionTable::probe_eval(
    const Position& pos, const size_t depth,
    const int alpha, const int beta) const
    -> std::optional<ProbedEval>
{
    if (const auto* record = find(pos);
        record != nullptr and record->searchedDepth >= depth) {
        switch (record->evalType) {
            using enum Record::EvalType;

            case Exact:
                return std::make_pair(record->eval, record->evalType);

            case Alpha: {
                if (record->eval <= alpha)
                    return std::make_pair(alpha, record->evalType);

                break;
            }

            case Beta: {
                if (record->eval >= beta)
                    return std::make_pair(beta, record->evalType);

                break;
            }

            default: std::unreachable();
        }
    }

    return std::nullopt;
}

void TranspositionTable::store(const Position& pos, const Record& record)
{
    if (const auto it = records.find(pos.hash);
        it != records.end()) {
        // this position was already stored in the table
        // keep the old evaluation if it was an exact one & the new one isn't,
        // or if the new evaluation is a lower depth than the old one

        auto& stored = it->second;

        const bool shouldReplace
            = record.searchedDepth > stored.searchedDepth
           or (stored.evalType != Record::EvalType::Exact
               and record.evalType == Record::EvalType::Exact);

        if (shouldReplace)
            stored = record;

        return;
    }

    // this position hasn't been searched before, add it to the table
    records.emplace(pos.hash, record);
}

} // namespace ben_bot
