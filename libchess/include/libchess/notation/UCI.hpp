/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for converting Move objects to and from
    UCI-format algebraic notation.

    @ingroup notation
 */

#pragma once

#include <libchess/moves/Move.hpp>
#include <string>
#include <string_view>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::notation {

using game::Position;
using moves::Move;

/** Returns the UCI-format algebraic notation for the given Move object.

    @ingroup notation
    @see from_uci()
 */
[[nodiscard]] std::string to_uci(const Move& move);

/** Parses the UCI-format algebraic notation string into a Move object.
    The current position is used to determine the type of the moved piece.

    @throws std::invalid_argument An exception will be thrown if a move cannot
    be parsed correctly from the input string.

    @ingroup notation
    @see from_uci()
 */
[[nodiscard]] Move from_uci(const Position& position, std::string_view text);

} // namespace chess::notation
