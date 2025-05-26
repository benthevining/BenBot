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

#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>
#include <utility>

/** This namespace contains classes for modeling moves.
    @ingroup moves
 */
namespace chess::moves {

using board::File;
using board::Rank;
using board::Square;
using pieces::Color;

using PieceType = pieces::Type;

/** This struct encodes information about a move.

    Moves are encoded as a starting and ending square, as well as a piece type.
    Castling is considered a king move.

    @ingroup moves

    @todo static creation func promotion()
    @todo is_en_passant()
    @todo static creation func en_passant()

    @todo std::formatter
    @todo std::hash
    @todo from_string()
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

    /** Returns true if this move is castling (in either direction). */
    [[nodiscard]] constexpr bool is_castling() const noexcept;
};

/// @ingroup moves
/// @{

/** Creates a move encoding kingside ("short") castling for the given color.

    @see castle_queenside()
    @relates Move
 */
[[nodiscard]] constexpr Move castle_kingside(Color color) noexcept;

/** Creates a move encoding queenside ("long") castling for the given color.

    @see castle_kingside()
    @relates Move
 */
[[nodiscard]] constexpr Move castle_queenside(Color color) noexcept;

/// @}

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

constexpr bool Move::is_castling() const noexcept
{
    if (piece != PieceType::King)
        return false;

    return std::cmp_greater(
        board::file_distance(from, to),
        1uz);
}

constexpr Move castle_kingside(const Color color) noexcept
{
    const auto rank = board::back_rank_for(color);

    return {
        .from  = Square { File::E, rank },
        .to    = Square { File::G, rank },
        .piece = PieceType::King
    };
}

constexpr Move castle_queenside(const Color color) noexcept
{
    const auto rank = board::back_rank_for(color);

    return {
        .from  = Square { File::E, rank },
        .to    = Square { File::C, rank },
        .piece = PieceType::King
    };
}

} // namespace chess::moves
