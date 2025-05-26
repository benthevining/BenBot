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

#include <cassert>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>
#include <stdexcept>
#include <string_view>
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

    En passant cannot be identified solely using the information in this struct;
    based on the starting and ending squares of the capturing pawn, an en passant
    capture appears just like any other pawn capture.

    This struct also does not directly identify if the move is a capture.

    @ingroup moves

    @todo std::hash
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

        @invariant This will never be PieceType::King or PieceType::Pawn.
     */
    std::optional<PieceType> promotedType;

    /** Returns true if this move is a promotion. */
    [[nodiscard]] constexpr bool is_promotion() const noexcept
    {
        return promotedType.has_value();
    }

    /** Returns true if this move is a promotion to a piece other than a queen. */
    [[nodiscard]] constexpr bool is_under_promotion() const noexcept;

    /** Returns true if this move is castling (in either direction). */
    [[nodiscard]] constexpr bool is_castling() const noexcept;

    /** Creates a move from a string in algebraic notation, such as "Nd4", "e8=Q",
        "O-O-O", etc.

        @throws std::invalid_argument An exception will be thrown if a move cannot be
        parsed correctly from the input string.

        @todo Move into Position class so that we can tell the starting square?
        @todo Deal with abbreviated pawn moves
     */
    [[nodiscard, gnu::const]] static constexpr Move from_string(
        std::string_view text, Color color);
};

/// @ingroup moves
/// @{

/** Creates a move encoding kingside ("short") castling for the given color.

    @see castle_queenside()
    @relates Move
 */
[[nodiscard, gnu::const]] constexpr Move castle_kingside(Color color) noexcept;

/** Creates a move encoding queenside ("long") castling for the given color.

    @see castle_kingside()
    @relates Move
 */
[[nodiscard, gnu::const]] constexpr Move castle_queenside(Color color) noexcept;

/** Creates a move encoding a pawn promotion.

    @relates Move
 */
[[nodiscard, gnu::const]] constexpr Move promotion(
    File file, Color color, PieceType promotedType = PieceType::Queen) noexcept;

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

constexpr bool Move::is_under_promotion() const noexcept
{
    return is_promotion()
        && *promotedType != PieceType::Queen;
}

constexpr bool Move::is_castling() const noexcept
{
    return piece == PieceType::King
        && std::cmp_greater(
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

constexpr Move promotion(
    const File file, const Color color, const PieceType promotedType) noexcept
{
    assert(promotedType != PieceType::King);
    assert(promotedType != PieceType::Pawn);

    const bool isWhite = color == Color::White;

    return {
        .from         = Square { file, isWhite ? Rank::Seven : Rank::Two },
        .to           = Square { file, isWhite ? Rank::Eight : Rank::One },
        .piece        = PieceType::Pawn,
        .promotedType = promotedType
    };
}

constexpr Move Move::from_string(
    const std::string_view text, const Color color)
{
    if (text == "O-O" || text == "0-0")
        return castle_kingside(color);

    if (text == "O-O-O" || text == "0-0-0")
        return castle_queenside(color);

    // promotion
    if (const auto eqSgnPos = text.find('=');
        eqSgnPos != std::string_view::npos) {
        const auto promotedType = pieces::from_string(text.substr(eqSgnPos + 1uz, 1uz));

        if (const auto xPos = text.find('x');
            xPos != std::string_view::npos) {
            // string is of form dxe8=Q
            return {
                .from         = board::file_from_char(text.at(xPos - 1uz)),
                .to           = Square::from_string(text.substr(eqSgnPos - 2uz, 2uz)),
                .piece        = PieceType::Pawn,
                .promotedType = promotedType
            };
        }

        // string is of form e8=Q
        return promotion(
            board::file_from_char(text.front()),
            color, promotedType);
    }

    // string is of the form Nc6 or Nxc6

    return {
        .from  = Square {}, // TODO: cannot tell the starting square
        .to    = Square::from_string(text.substr(text.length() - 2uz)),
        .piece = pieces::from_string(text.substr(0uz, 1uz))
    };

    throw std::invalid_argument {
        std::format("Cannot parse Move from invalid input string: {}", text)
    };
}

} // namespace chess::moves
