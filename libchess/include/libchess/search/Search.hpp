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

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/moves/Move.hpp>

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

class TranspositionTable;

/** Finds the best move for the side to move in the given position.

    @throws std::invalid_argument An exception will be thrown if there are
    no legal moves for the side to move in the given position.

    @ingroup search
 */
[[nodiscard]] Move find_best_move(
    const Position&     position,
    TranspositionTable& transTable,
    size_t              searchDepth = 4uz);

} // namespace chess::search
