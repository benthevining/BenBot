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

using PieceType      = pieces::Type;
using MaybePieceType = std::optional<PieceType>;

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
    @todo Flags for en passant, maybe isCapture/isCheck, isCastling
 */
struct Move final {
    /** Creates a null move. */
    constexpr Move() = default;

    /** Creates a move. */
    constexpr Move(
        Square start, Square end,
        PieceType      type,
        MaybePieceType promotedType = std::nullopt);

    /** Returns the starting square of the moving piece.
        In the case of en passant, this is the square that the capturing pawn started on.
        In the case of castling, this is the square that the king started on.
     */
    [[nodiscard]] constexpr auto from() const noexcept -> Square;

    /** Returns the ending square of the moving piece.
        In the case of en passant, this is the square that the capturing pawn lands on.
        In the case of castling, this is the square that the king ends on.
     */
    [[nodiscard]] constexpr auto to() const noexcept -> Square;

    /** Returns the type of the moving piece.
        For a promotion, this is pawn; for castling, this is king.
     */
    [[nodiscard]] constexpr auto piece() const noexcept -> PieceType;

    /** Returns the promoted-to type, or ``nullopt`` if this move is not a promotion. */
    [[nodiscard]] constexpr auto promoted_type() const noexcept -> MaybePieceType;

    /** Returns a hash code for this move. */
    [[nodiscard]] constexpr auto hash() const noexcept -> size_t;

    /** Returns true if this is a null move. */
    [[nodiscard]] auto is_null() const noexcept -> bool { return std::cmp_equal(data, 0); }

    /** Returns true if this move is a promotion. */
    [[nodiscard]] constexpr auto is_promotion() const noexcept -> bool;

    /** Returns true if this move is a promotion to a piece other than a queen. */
    [[nodiscard]] constexpr auto is_under_promotion() const noexcept -> bool;

    /** Returns true if this move is castling (in either direction). */
    [[nodiscard]] constexpr auto is_castling() const noexcept -> bool;

    constexpr auto operator==(const Move&) const noexcept -> bool = default;

private:
    using Integer = std::uint16_t;

    [[nodiscard]] constexpr auto get_flags() const noexcept -> Integer;

    // A move needs 16 bits to be stored
    // Square indices are 0-63, so we need 6 bits to store them
    //
    // bit  0- 5: destination square index
    // bit  6-11: origin square index
    // bit 12-15: integer (0-15) that encodes both moving type & promoted type
    //          : 0-5 gives moving piece type & implies the move is not a promotion
    //          : 6-11 gives promoted piece type, implies the moving type is pawn
    //          : get_flags() declared above returns this integer
    //
    // Special case is a null move, this integer will be 0
    Integer data { UINT16_C(0) };
};

/** Provides a strong ordering of moves. This can be useful for sorting lists of moves.
    @ingroup moves
    @relates Move
 */
[[nodiscard, gnu::const]] constexpr auto operator<=>(const Move& first, const Move& second) noexcept
    -> std::strong_ordering
{
    return first.hash() <=> second.hash();
}

/// @ingroup moves
/// @{

/** Creates a move encoding kingside ("short") castling for the given color.

    @see castle_queenside()
    @relates Move
 */
[[nodiscard, gnu::const]] constexpr auto castle_kingside(Color color) noexcept -> Move;

/** Creates a move encoding queenside ("long") castling for the given color.

    @see castle_kingside()
    @relates Move
 */
[[nodiscard, gnu::const]] constexpr auto castle_queenside(Color color) noexcept -> Move;

/** Creates a move encoding a pawn promotion.
    This function creates simple promotions that don't involve captures.

    @relates Move
 */
[[nodiscard, gnu::const]] constexpr auto promotion(
    File file, Color color, PieceType promotedType = PieceType::Queen) noexcept -> Move;

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

#ifndef DOXYGEN
namespace detail {
    using Integer = std::uint16_t;

    inline constexpr auto LOWEST_SIX_BITS_MASK = 63uz;

    inline constexpr Integer START_SQUARE_OFFSET = UINT16_C(6);
    inline constexpr Integer FLAGS_OFFSET        = UINT16_C(12);

    inline constexpr auto PROMOTED_TYPE_OFFSET_WITHIN_FLAGS = 6uz;

    [[nodiscard, gnu::const]] constexpr auto pack_fields(
        const Square start, const Square end, const PieceType type,
        const MaybePieceType promotedType) noexcept
        -> Integer
    {
        const auto flags = promotedType
                               .transform([](const PieceType prom) {
                                   return PROMOTED_TYPE_OFFSET_WITHIN_FLAGS + std::to_underlying(prom);
                               })
                               .value_or(std::to_underlying(type));

        return static_cast<Integer>(
            static_cast<Integer>(end.index())
            + (static_cast<Integer>(start.index()) << START_SQUARE_OFFSET)
            + (static_cast<Integer>(flags) << FLAGS_OFFSET));
    }
} // namespace detail
#endif

constexpr Move::Move(
    const Square start, const Square end,
    const PieceType      type,
    const MaybePieceType promotedType)
    : data { detail::pack_fields(start, end, type, promotedType) }
{
#ifndef NDEBUG
    if (promotedType.has_value()) {
        assert(type == PieceType::Pawn);
        assert(promotedType != PieceType::King);
        assert(promotedType != PieceType::Pawn);
    }
#endif
}

constexpr auto Move::get_flags() const noexcept -> Integer
{
    static constexpr auto LOWEST_FOUR_BITS_MASK = 15uz;

    return (data >> detail::FLAGS_OFFSET) & LOWEST_FOUR_BITS_MASK;
}

constexpr auto Move::from() const noexcept -> Square
{
    return Square::from_index((data >> detail::START_SQUARE_OFFSET) & detail::LOWEST_SIX_BITS_MASK);
}

constexpr auto Move::to() const noexcept -> Square
{
    return Square::from_index(data & detail::LOWEST_SIX_BITS_MASK);
}

constexpr auto Move::piece() const noexcept -> PieceType
{
    const auto flags = get_flags();

    if (std::cmp_greater_equal(flags, detail::PROMOTED_TYPE_OFFSET_WITHIN_FLAGS))
        return PieceType::Pawn;

    return static_cast<PieceType>(flags);
}

constexpr auto Move::promoted_type() const noexcept -> MaybePieceType
{
    const auto flags = get_flags();

    if (std::cmp_less(flags, detail::PROMOTED_TYPE_OFFSET_WITHIN_FLAGS))
        return std::nullopt;

    return static_cast<PieceType>(flags - detail::PROMOTED_TYPE_OFFSET_WITHIN_FLAGS);
}

constexpr auto Move::is_promotion() const noexcept -> bool
{
    return std::cmp_greater_equal(
        get_flags(), detail::PROMOTED_TYPE_OFFSET_WITHIN_FLAGS);
}

constexpr auto Move::is_under_promotion() const noexcept -> bool
{
    return promoted_type()
        .transform([](const PieceType type) { return type != PieceType::Queen; })
        .value_or(false);
}

constexpr auto Move::is_castling() const noexcept -> bool
{
    return piece() == PieceType::King
       and std::cmp_greater(
           file_distance(from(), to()),
           1uz);
}

constexpr auto Move::hash() const noexcept -> size_t
{
    // based on a congruential pseudo-random number generator
    return (static_cast<size_t>(data) * 6364136223846793005uz) + 1442695040888963407uz;
}

constexpr auto castle_kingside(const Color color) noexcept -> Move
{
    const auto rank = board::back_rank_for(color);

    return {
        Square { .file = File::E, .rank = rank },
        Square { .file = File::G, .rank = rank },
        PieceType::King
    };
}

constexpr auto castle_queenside(const Color color) noexcept -> Move
{
    const auto rank = board::back_rank_for(color);

    return {
        Square { .file = File::E, .rank = rank },
        Square { .file = File::C, .rank = rank },
        PieceType::King
    };
}

constexpr auto promotion(
    const File file, const Color color, const PieceType promotedType) noexcept
    -> Move
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
