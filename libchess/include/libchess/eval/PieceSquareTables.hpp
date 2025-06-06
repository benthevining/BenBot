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
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::eval {

using PieceType = pieces::Type;

/** Returns an evaluation value for the given piece type on
    the given square.

    @ingroup eval
 */
[[nodiscard, gnu::const]] Value get_piece_square_value(
    PieceType piece, const board::Square& square) noexcept;

} // namespace chess::eval
