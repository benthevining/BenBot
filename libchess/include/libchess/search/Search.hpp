/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup search Search
    Functions for searching to find the best move.
 */

/** @file
    This file defines the NegaMax function.
    @ingroup search
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/moves/Move.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <optional>
#include <vector>

namespace chess::game {
struct Position;
} // namespace chess::game

/** This namespace contains functions related to searching the
    move tree for the principal variation.

    @ingroup search
 */
namespace chess::search {

using game::Position;
using moves::Move;
using std::size_t;

using Milliseconds = std::chrono::milliseconds;

/** This struct encapsulates the parameters to the search algorithm.

    @ingroup search
    @see find_best_move()
 */
struct Options final {
    /** The root position to be searched. */
    Position position;

    /** The maximum search depth (in plies). */
    size_t depth { 4uz };

    /** The maximum search time. */
    std::optional<Milliseconds> searchTime;

    /** Search only this many nodes. Setting this value too low can
        really emphasize the effects of our move ordering algorithm.
     */
    std::optional<size_t> maxNodes;

    /** Restrict the search to only these moves.
        If this is empty, all legal moves in the position will be searched.
     */
    std::vector<Move> movesToSearch;
};

/** Finds the best move for the side to move in the given position.

    ``exitFlag`` will be queried after each iteration of the iterative
    deepening loop, and the search will exit if the flag has been set to true.

    See the Context and Thread classes for higher-level encapsulations of
    search functionality.

    @throws std::invalid_argument An exception will be thrown if there are
    no legal moves for the side to move in the given position.

    @ingroup search
    @see Context, Options, Thread
 */
[[nodiscard]] Move find_best_move(
    const Options&          options,
    TranspositionTable&     transTable,
    const std::atomic_bool& exitFlag = false);

/** This struct encapsulates everything needed to perform a search.
    You can keep one of these alive between searches by simply updating
    the options and then calling ``search()`` again.

    The Thread class provides the capability to run the search on a background thread.

    @ingroup search
    @see find_best_move(), Thread
 */
struct Context final {
    /** The options to use for the search. */
    Options options;

    /** The transposition table used for the search.
        Results are persistent between successive searches.
     */
    TranspositionTable transTable;

    /** An exit flag used for this search.
        This can be set to true while the ``search()`` method is running,
        which will cause it to abort at the next opportunity. This is reset
        to false at the start of each ``search()`` invocation.
     */
    std::atomic_bool exitFlag { false };

    /** Performs a search.
        @see find_best_move()
     */
    [[nodiscard]] Move search()
    {
        exitFlag.store(false);

        return find_best_move(options, transTable, exitFlag);
    }
};

} // namespace chess::search
