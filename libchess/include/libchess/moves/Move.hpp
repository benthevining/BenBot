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

/** @defgroup moves Moves
    Classes for modeling moves.
    In the search tree, positions are nodes and moves are edges between them.

    @ingroup libchess
 */

/** @file
    This file defines the Move struct.
    @ingroup moves
 */

#pragma once

#include <cassert>
#include <compare>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint32_t
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

using std::size_t;

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

    Null moves are supported; a null move essentially indicates swapping the side
    to move and incrementing the halfmove counter without changing any piece positions.

    @ingroup moves
 */
struct Move final {
    /** Integer type used to hold move data in a packed format. */
    using Integer = std::uint32_t;

    /** Creates a null move. */
    constexpr Move() = default;

    /** Initializes a move from the raw packed data format. */
    explicit constexpr Move(const Integer rawData)
        : data { rawData }
    {
    }

    /** Creates a non-promotion move. */
    constexpr Move(
        const Square start, const Square end,
        const PieceType type)
        : data { pack_fields(start, end, type, static_cast<PieceType>(0)) }
    {
    }

    /** Creates a promotion move. */
    constexpr Move(
        const Square start, const Square end,
        const PieceType type, const PieceType promotedType)
        : data { pack_fields(start, end, type, promotedType) }
    {
        assert(promotedType != PieceType::King);
        assert(promotedType != PieceType::Pawn);
    }

    /** Returns the starting square of the moving piece.
        In the case of en passant, this is the square that the capturing pawn started on.
        In the case of castling, this is the square that the king started on.
     */
    [[nodiscard]] constexpr Square from() const noexcept
    {
        return Square::from_index((data >> START_SQUARE_OFFSET) & LOWEST_SIX_BITS_MASK);
    }

    /** Returns the ending square of the moving piece.
        In the case of en passant, this is the square that the capturing pawn lands on.
        In the case of castling, this is the square that the king ends on.
     */
    [[nodiscard]] constexpr Square to() const noexcept
    {
        return Square::from_index(data & LOWEST_SIX_BITS_MASK);
    }

    /** Returns the type of the moving piece. For a promotion, this is pawn;
        for castling, this is king.
     */
    [[nodiscard]] constexpr PieceType piece() const noexcept
    {
        return static_cast<PieceType>((data >> MOVING_TYPE_OFFSET) & LOWEST_THREE_BITS_MASK);
    }

    /** Returns the promoted-to type, or ``nullopt`` if this move is not a promotion. */
    [[nodiscard]] constexpr std::optional<PieceType> promoted_type() const noexcept;

    /** Returns the raw packed data format. */
    [[nodiscard]] constexpr Integer raw() const noexcept { return data; }

    /** Returns a hash code for this move. */
    [[nodiscard]] constexpr size_t hash() const noexcept
    {
        // based on a congruential pseudo-random number generator
        return static_cast<std::uint64_t>(data) * 6364136223846793005ULL + 1442695040888963407ULL;
    }

    /** Returns true if this is a null move. */
    [[nodiscard]] bool is_null() const noexcept { return std::cmp_equal(data, 0); }

    /** Returns true if this move is a promotion. */
    [[nodiscard]] constexpr bool is_promotion() const noexcept;

    /** Returns true if this move is a promotion to a piece other than a queen. */
    [[nodiscard]] constexpr bool is_under_promotion() const noexcept;

    /** Returns true if this move is castling (in either direction). */
    [[nodiscard]] constexpr bool is_castling() const noexcept;

    constexpr bool operator==(const Move&) const noexcept = default;

private:
    static constexpr auto LOWEST_SIX_BITS_MASK   = 0x3F;
    static constexpr auto LOWEST_THREE_BITS_MASK = 0x7;

    static constexpr auto START_SQUARE_OFFSET  = 6uz;
    static constexpr auto MOVING_TYPE_OFFSET   = 12uz;
    static constexpr auto PROMOTED_TYPE_OFFSET = 15uz;

    [[nodiscard, gnu::const]] static constexpr Integer pack_fields(
        Square start, Square end,
        PieceType type, PieceType promotedType) noexcept;

    // A move needs 18 bits to be stored
    // Square indices are 0-63, so we need 6 bits to store them
    // Piece types are 0-5, so they need 3 bits
    //
    // bit  0- 5: destination square index
    // bit  6-11: origin square index
    // bit 12-14: moving piece type
    // bit 15-17: promoted piece type (this could be stored in 2 bits, the total would then be 17)
    //
    // Special case is a null move, this integer will be 0
    Integer data { 0 };
};

/** Provides a strong ordering of moves. This can be useful for sorting lists of moves.
    @ingroup moves
    @relates Move
 */
[[nodiscard, gnu::const]] constexpr std::strong_ordering operator<=>(const Move& first, const Move& second) noexcept
{
    return first.hash() <=> second.hash();
}

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
    This function creates simple promotions that don't involve captures.

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

constexpr auto Move::pack_fields(
    const Square start, const Square end,
    const PieceType type, const PieceType promotedType) noexcept -> Integer
{
    return static_cast<Integer>(end.index())
         + (static_cast<Integer>(start.index()) << START_SQUARE_OFFSET)
         + (static_cast<Integer>(std::to_underlying(type)) << MOVING_TYPE_OFFSET)
         + (static_cast<Integer>(std::to_underlying(promotedType)) << PROMOTED_TYPE_OFFSET);
}

constexpr std::optional<PieceType> Move::promoted_type() const noexcept
{
    if (not is_promotion())
        return std::nullopt;

    return static_cast<PieceType>(
        (data >> PROMOTED_TYPE_OFFSET) & LOWEST_THREE_BITS_MASK);
}

constexpr bool Move::is_promotion() const noexcept
{
    const auto destRank = to().rank;

    return (destRank == Rank::One or destRank == Rank::Eight)
       and piece() == PieceType::Pawn;
}

constexpr bool Move::is_under_promotion() const noexcept
{
    const auto prom = promoted_type();

    return prom.has_value()
       and prom.value() != PieceType::Queen;
}

constexpr bool Move::is_castling() const noexcept
{
    return piece() == PieceType::King
       and std::cmp_greater(
           file_distance(from(), to()),
           1uz);
}

constexpr Move castle_kingside(const Color color) noexcept
{
    const auto rank = board::back_rank_for(color);

    return {
        Square { .file = File::E, .rank = rank },
        Square { .file = File::G, .rank = rank },
        PieceType::King
    };
}

constexpr Move castle_queenside(const Color color) noexcept
{
    const auto rank = board::back_rank_for(color);

    return {
        Square { .file = File::E, .rank = rank },
        Square { .file = File::C, .rank = rank },
        PieceType::King
    };
}

constexpr Move promotion(
    const File file, const Color color, const PieceType promotedType) noexcept
{
    assert(promotedType != PieceType::King);
    assert(promotedType != PieceType::Pawn);

    const bool isWhite = color == Color::White;

    return {
        Square { .file = file, .rank = isWhite ? Rank::Seven : Rank::Two },
        Square { .file = file, .rank = isWhite ? Rank::Eight : Rank::One },
        PieceType::Pawn,
        promotedType
    };
}

} // namespace chess::moves
