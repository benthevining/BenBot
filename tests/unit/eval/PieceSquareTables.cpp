/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/eval/PieceSquareTables.hpp>
#include <libchess/notation/FEN.hpp>

static constexpr auto TAGS { "[eval][piece-square-tables]" };

using chess::eval::score_piece_placement;
using chess::notation::from_fen;

// pawns
// knights
// bishops
// rooks
// queens
// king
