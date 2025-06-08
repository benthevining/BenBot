/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for "magic bitboard" move generation for sliding pieces.
    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Square.hpp>

/** This namespace contains functions for "magic bitboard" move generation for sliding pieces.
    The generated moves are pseudo-legal, not strictly legal, because they do not consider
    whether the king would be left in check. The returned move sets do not include the starting
    square.

    @ingroup moves
 */
namespace chess::moves::magics {

using board::Bitboard;
using board::Square;

/// @ingroup moves
/// @{

/** Calculates all pseudo-legal bishop moves.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard]] Bitboard bishop(
    const Square& bishopPos, Bitboard occupiedSquares, Bitboard friendlyPieces);

/** Calculates all pseudo-legal rook moves.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard]] Bitboard rook(
    const Square& rookPos, Bitboard occupiedSquares, Bitboard friendlyPieces);

/** Calculates all pseudo-legal queen moves.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard]] Bitboard queen(
    const Square& queenPos, Bitboard occupiedSquares, Bitboard friendlyPieces);

/// @}

} // namespace chess::moves::magics
