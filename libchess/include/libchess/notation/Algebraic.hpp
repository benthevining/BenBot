/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for converting Move objects
    to/from algebraic notation.

    @ingroup notation
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <string>
#include <string_view>

namespace chess::notation {

using game::Position;
using moves::Move;

/** Returns the algebraic notation for the given Move object.
    The current state of the position is used to determine whether the move is
    a capture, and the pieces of the side to move are used for disambiguation
    if necessary.

    @ingroup notation
    @see from_alg()
 */
[[nodiscard]] std::string to_alg(const Position& position, const Move& move);

/** Creates a move from a string in algebraic notation, such as "Nd4", "e8=Q",
    "O-O-O", etc.

    @ingroup notation
    @see to_alg()
 */
[[nodiscard]] Move from_alg(const Position& position, std::string_view text);

} // namespace chess::notation
