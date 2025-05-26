/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup moves Moves
    Classes for modeling moves.
    In the search tree, positions are nodes and moves are edges between them.
 */

/** @file
    This file defines the Move struct.
    @ingroup moves
 */

#pragma once

#include <libchess/board/Square.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>

/** This namespace contains classes for modeling moves.
    @ingroup moves
 */
namespace chess::moves {

using board::Square;

using PieceType = pieces::Type;

/** This struct encodes information about a move.

    Moves are encoded as a starting and ending square, as well as a piece type.
    Castling is considered a king move.

    @ingroup moves

    @todo is_castling()
    @todo is_en_passant()
    @todo static creation funcs castle_queenside(), castle_kingside(), en_passant()
 */
struct Move final {
    /** The starting square of the moving piece.
        In the case of en passant, this is the square that the capturing pawn started on.
        In the case of castling, this is the square that the king started on.

        @invariant ``from != to``
     */
    Square from;

    /** The ending square of the moving piece.
        In the case of en passant, this is the square that the capturing pawn lands on.
        In the case of castling, this is the square that the king ends on.

        @invariant ``to != from``
     */
    Square to;

    /** The type of the moving piece.
        In the case of castling, this will be PieceType::King.
     */
    PieceType piece;

    /** If this move is a promotion, this is the type of the promoted piece.
        If this move is not a promotion, this will be ``nullopt``.

        @invariant This will never be PieceType::King.
     */
    std::optional<PieceType> promotedType;

    /** Returns true if this move is a promotion. */
    [[nodiscard]] constexpr bool is_promotion() const noexcept
    {
        return promotedType.has_value();
    }
};

} // namespace chess::moves
