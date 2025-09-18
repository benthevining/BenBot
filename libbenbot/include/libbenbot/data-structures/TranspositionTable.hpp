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

/** @file
    This file defines the transposition table data structure.
    @ingroup benbot_data_structures
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>
#include <unordered_map>
#include <utility>

namespace ben_bot {

using chess::game::Position;
using chess::moves::Move;
using std::size_t;

/** The transposition table data structure.

    @ingroup benbot_data_structures
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

        constexpr bool operator==(const Record& other) const noexcept = default;
    };

    /** Retrieves the stored record for the given position,
        or nullptr if the given position isn't in the table.
     */
    [[nodiscard]] const Record* find(const Position& pos) const;

    /** Represents a probed evaluation from the table.
        This is a pair of the evaluation value and the value type.
     */
    using ProbedEval = std::pair<int, Record::EvalType>;

    /** Similar to ``find()``, this function instead probes for an
        evaluation value of the given position, searched to at least
        the given depth and honoring the alpha/beta cutoff values.

        Returns pair of the evaluation value and the value type.
     */
    [[nodiscard]] std::optional<ProbedEval> probe_eval(
        const Position& pos, size_t depth, int alpha, int beta) const;

    /** Returns the opponent's best response to the given move, if one
        is recorded.
     */
    [[nodiscard]] std::optional<Move> get_best_response(
        const Position& pos, const Move& move) const;

    /** Stores a record for a given position. */
    void store(const Position& pos, const Record& record);

    /** Clears the contents of the table. */
    void clear() noexcept { records.clear(); }

private:
    std::unordered_map<Position::Hash, Record> records;
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

inline auto TranspositionTable::find(const Position& pos) const -> const Record*
{
    if (const auto it = records.find(pos.hash);
        it != records.end()) {
        return &it->second;
    }

    return nullptr;
}

inline std::optional<Move> TranspositionTable::get_best_response(
    const Position& pos, const Move& move) const
{
    if (const auto* record = find(after_move(pos, move)))
        return record->bestMove;

    return std::nullopt;
}

} // namespace ben_bot
