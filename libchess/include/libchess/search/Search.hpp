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
#include <optional>

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

class TranspositionTable;

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

    // TODO: max nodes
};

/** Finds the best move for the side to move in the given position.

    ``exitFlag`` will be queried after each iteration of the iterative
    deepening loop, and the search will exit if the flag has been set to true.

    See the Thread class for a higher-level encapsulation of search functionality.

    @throws std::invalid_argument An exception will be thrown if there are
    no legal moves for the side to move in the given position.

    @ingroup search
    @see Options, Thread
 */
[[nodiscard]] Move find_best_move(
    const Options&          options,
    TranspositionTable&     transTable,
    const std::atomic_bool& exitFlag = false);

} // namespace chess::search
