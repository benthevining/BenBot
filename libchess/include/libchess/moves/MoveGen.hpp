/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

/** @file
    This file provides move generation functions.
    @ingroup moves
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveRange.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::moves {

using game::Position;
using PieceType = pieces::Type;

/// @ingroup moves
/// @{

/** Generates a list of all legal moves for the side to move in the given position.
    If the side to move is in checkmate or stalemate, this returns an empty list.
    The list of moves is not sorted in any particular manner.

    @see generate_for()
 */
template <bool CapturesOnly = false>
[[nodiscard]] MoveRange generate(const Position& position);

/** Generates a list of all legal moves for only the given piece type in the given position.

    Generating King moves will include castling. Generating pawn moves will include all
    pushes, double pushes, captures, promotions, and en passant captures.

    @see generate()
 */
template <bool CapturesOnly = false>
[[nodiscard]] MoveRange generate_for(
    const Position& position, PieceType piece);

/** Returns true if the side to move has any legal moves in the given position. */
[[nodiscard]] bool any_legal_moves(const Position& position);

/// @}

} // namespace chess::moves
