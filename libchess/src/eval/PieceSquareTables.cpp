/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/eval/PieceSquareTables.hpp>

namespace chess::eval {

// tables for:
// black pawns
// white pawns
// knights
// bishops
// rooks
// queens
// white king
// black king

Value get_piece_square_value(
    const PieceType piece, Color color, const board::Square& square) noexcept
{
}

} // namespace chess::eval
