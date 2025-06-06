/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines piece-square tables.
    @ingroup eval
 */

#pragma once

#include <libchess/board/Square.hpp>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::eval {

using PieceType = pieces::Type;
using game::Position;
using pieces::Color;

/** Returns an evaluation value for the given piece type on
    the given square.

    @ingroup eval
 */
[[nodiscard, gnu::const]] Value get_piece_square_value(
    PieceType piece, Color color, const board::Square& square) noexcept;

/** Returns an aggregate score for the placement of both sides' pieces,
    relative to the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] Value score_piece_placement(const Position& position) noexcept;

} // namespace chess::eval
