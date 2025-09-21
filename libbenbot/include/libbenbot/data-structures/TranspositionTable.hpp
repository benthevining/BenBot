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
#include <utility>

namespace ben_bot {

namespace search {
    struct Bounds;
} // namespace search

using chess::game::Position;
using chess::moves::Move;
using std::size_t;

/** The transposition table data structure.

    @ingroup benbot_data_structures
 */
class TranspositionTable final {
public:
    using Hash = Position::Hash;

    /** A record of a previously searched position. */
    struct Record final {
        /** The depth that the position was searched to. */
        size_t searchedDepth { 0uz }; // empty slots are marked with a depth of 0

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

        Hash hash { 0 }; // TODO: move into separate internal structure

        constexpr bool operator==(const Record& other) const noexcept = default;
    };

    TranspositionTable();

    ~TranspositionTable();

    TranspositionTable(const TranspositionTable&)            = delete;
    TranspositionTable& operator=(const TranspositionTable&) = delete;

    TranspositionTable(TranspositionTable&&)            = delete;
    TranspositionTable& operator=(TranspositionTable&&) = delete;

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
        the given depth and honoring the given search bounds.

        Returns pair of the evaluation value and the value type.
     */
    [[nodiscard]] std::optional<ProbedEval> probe_eval(
        const Position& pos, size_t depth, const search::Bounds& bounds) const;

    /** Returns the opponent's best response to the given move, if one
        is recorded.
     */
    [[nodiscard]] std::optional<Move> get_best_response(
        const Position& pos, const Move& move) const;

    /** Stores a record for a given position. */
    void store(const Position& pos, Record record);

    /** Clears the contents of the table. Note that no memory is freed. */
    void clear();

    /** Resizes the table to a given size in megabytes.

        @throws std::bad_alloc An exception is thrown if ``sizeMB`` is 0 or
        if the allocation fails.
     */
    void resize(size_t sizeMB);

    /** Returns an estimate of the percentage of entries (permille) that
        have been written to during this search.
     */
    [[nodiscard]] size_t hashfull() const;

    /** This must be called at the beginning of a new search to keep
        track of entry aging.
     */
    void new_search() noexcept;

private:
    // this is the hash function
    [[nodiscard]] Record* first_entry(Position::Hash key) const noexcept;

    struct Cluster;

    Cluster* table { nullptr };

    size_t clusterCount { 0uz };
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

inline std::optional<Move> TranspositionTable::get_best_response(
    const Position& pos, const Move& move) const
{
    if (const auto* record = find(after_move(pos, move)))
        return record->bestMove;

    return std::nullopt;
}

} // namespace ben_bot
