/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the transposition table data structure.
    @ingroup search
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>
#include <unordered_map>

namespace chess::search {

using game::Position;
using moves::Move;
using std::size_t;

/** The transposition table data structure.

    @ingroup search

    @todo maximum size
    @todo replacement scheme
 */
class TranspositionTable final {
public:
    /** A record of a previously searched position. */
    struct Record final {
        /** The depth that the position was searched to. */
        size_t searchedDepth { 0uz };

        /** The evaluation of this position.
            See ``evalType`` to determine the exact meaning of this value.
         */
        int eval { 0 };

        /** This enumeration defines types of evaluation values that
            different nodes in the search tree may be assigned.
         */
        enum class EvalType : std::uint_least8_t {
            Exact, ///< Indicates that the ``eval`` value is an exact evaluation. This also indicates that this is a PV node.
            Alpha, ///< Indicate that the ``eval`` value is a maximum evaluation; for example, if ``eval`` is 16, this means that the evaluation of this node was at most 16.
            Beta   ///< Indicates that the ``eval`` is a minimum evaluation; for example, if ``eval`` is 16, this means that the evaluation of this node was at least 16.
        };

        /** Gives the exact meaning of the ``eval`` value. */
        EvalType evalType { EvalType::Alpha };

        /** If a conclusive best move was found in this position, it
            is stored here. Sometimes this may be ``nullopt`` if everything
            failed low (i.e. ``score <= alpha``).
         */
        std::optional<Move> bestMove;

        // TODO: maybe also cache legal moves
    };

    /** Retrieves the stored record for the given position,
        or nullptr if the given position isn't in the table.
     */
    [[nodiscard]] const Record* find(const Position& pos) const
    {
        if (const auto it = records.find(pos.hash);
            it != records.end()) {
            return &it->second;
        }

        return nullptr;
    }

    /** Similar to ``find()``, this function instead probes for an
        evaluation value of the given position, searched to at least
        the given depth and honoring the alpha/beta cutoff values.
     */
    [[nodiscard]] std::optional<int> probe_eval(
        const Position& pos, size_t depth, int alpha, int beta) const;

    /** Stores a record for a given position. */
    void store(const Position& pos, const Record& record)
    {
        // TODO: if key was in table, need to determine whether it should be overwritten
        records[pos.hash] = record;
    }

private:
    std::unordered_map<std::uint64_t, Record> records;
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

inline std::optional<int> TranspositionTable::probe_eval(
    const Position& pos, const size_t depth, const int alpha, const int beta) const
{
    if (const auto* record = find(pos);
        record != nullptr && record->searchedDepth >= depth) {
        switch (record->evalType) {
            using enum Record::EvalType;

            case Exact:
                return record->eval;

            case Alpha: {
                if (record->eval <= alpha)
                    return alpha;

                break;
            }

            default: { // Beta
                if (record->eval >= beta)
                    return beta;
            }
        }
    }

    return std::nullopt;
}

} // namespace chess::search
