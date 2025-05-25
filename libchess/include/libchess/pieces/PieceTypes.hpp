/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup pieces Piece types
    Classes for working with the various chess piece types, and associated information.
 */

/** @file
    This file defines some constants for encoding information about the various piece types.
    @ingroup pieces
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint_fast8_t
#include <libchess/pieces/Colors.hpp>
#include <string_view>

/** This namespace contains classes for encoding information about the various chess piece types.
    @ingroup pieces
 */
namespace chess::pieces {

using std::size_t;

/** This enum identifies the different types of pieces.

    Black & white pawns have different enums to encode the differing movement direction;
    therefore, a Type enum on its own completely encodes the movement mechanics
    (except for castling or en passant).

    @ingroup pieces
    @see utf8 values

    @todo from_string()
    @todo std::formatter
 */
enum class Type : std::uint_fast8_t {
    WhitePawn, ///< A White pawn.
    BlackPawn, ///< A Black pawn.
    Knight,    ///< A knight.
    Bishop,    ///< A bishop.
    Rook,      ///< A rook.
    Queen,     ///< A queen.
    King       ///< A king.
};

/// @ingroup pieces
/// @{

/** Returns true if ``type`` is ``Knight`` or ``Bishop``.
    @see is_heavy()
 */
[[nodiscard, gnu::const]] constexpr bool is_minor(const Type type) noexcept
{
    return type == Type::Knight || type == Type::Bishop;
}

/** Returns true if ``type`` is ``Rook`` or ``Queen``.
    @see is_minor()
 */
[[nodiscard, gnu::const]] constexpr bool is_heavy(const Type type) noexcept
{
    return type == Type::Rook || type == Type::Queen;
}

/** Returns true if ``type`` is ``Bishop``, ``Rook``, or ``Queen``. */
[[nodiscard, gnu::const]] constexpr bool is_sliding(const Type type) noexcept
{
    return type == Type::Bishop || type == Type::Rook || type == Type::Queen;
}

/// @}

/** This namespace contains constants encoding the material values of the various piece types.
    The king is not assigned a material value, as it can never be legally captured in a non-checkmated position.
    @ingroup pieces
 */
namespace values {

    /// @ingroup pieces
    /// @{

    /** Returns the material value of a pawn (1). */
    [[nodiscard, gnu::const]] static consteval size_t pawn() noexcept
    {
        return 1uz;
    }

    /** Returns the material value of a knight (3). */
    [[nodiscard, gnu::const]] static consteval size_t knight() noexcept
    {
        return 3uz;
    }

    /** Returns the material value of a bishop (4). */
    [[nodiscard, gnu::const]] static consteval size_t bishop() noexcept
    {
        return 4uz;
    }

    /** Returns the material value of a rook (5). */
    [[nodiscard, gnu::const]] static consteval size_t rook() noexcept
    {
        return 5uz;
    }

    /** Returns the material value of a queen (9). */
    [[nodiscard, gnu::const]] static consteval size_t queen() noexcept
    {
        return 9uz;
    }

    /// @}

} // namespace values

/** This namespace contains some UTF8 encodings of chess piece symbols.
    @ingroup pieces
 */
namespace utf8 {

    /** This namespace contains some UTF8 encoding of White chess piece symbols.

        @ingroup pieces
        @see black
     */
    namespace white {

        /// @ingroup pieces
        /// @{

        /** Returns a UTF8-encoded symbol for a White pawn. */
        [[nodiscard, gnu::const]] consteval std::string_view pawn() noexcept
        {
            return "\xE2\x99\x99";
        }

        /** Returns a UTF8-encoded symbol for a White knight. */
        [[nodiscard, gnu::const]] consteval std::string_view knight() noexcept
        {
            return "\xE2\x99\x98";
        }

        /** Returns a UTF8-encoded symbol for a White bishop. */
        [[nodiscard, gnu::const]] consteval std::string_view bishop() noexcept
        {
            return "\xE2\x99\x97";
        }

        /** Returns a UTF8-encoded symbol for a White rook. */
        [[nodiscard, gnu::const]] consteval std::string_view rook() noexcept
        {
            return "\xE2\x99\x96";
        }

        /** Returns a UTF8-encoded symbol for a White queen. */
        [[nodiscard, gnu::const]] consteval std::string_view queen() noexcept
        {
            return "\xE2\x99\x95";
        }

        /** Returns a UTF8-encoded symbol for a White king. */
        [[nodiscard, gnu::const]] consteval std::string_view king() noexcept
        {
            return "\xE2\x99\x94";
        }

        /// @}

    } // namespace white

    /** This namespace contains some UTF8 encoding of Black chess piece symbols.

        @ingroup pieces
        @see white
     */
    namespace black {

        /// @ingroup pieces
        /// @{

        /** Returns a UTF8-encoded symbol for a Black pawn. */
        [[nodiscard, gnu::const]] consteval std::string_view pawn() noexcept
        {
            return "\xE2\x99\x9F";
        }

        /** Returns a UTF8-encoded symbol for a Black knight. */
        [[nodiscard, gnu::const]] consteval std::string_view knight() noexcept
        {
            return "\xE2\x99\x9E";
        }

        /** Returns a UTF8-encoded symbol for a Black bishop. */
        [[nodiscard, gnu::const]] consteval std::string_view bishop() noexcept
        {
            return "\xE2\x99\x9D";
        }

        /** Returns a UTF8-encoded symbol for a Black rook. */
        [[nodiscard, gnu::const]] consteval std::string_view rook() noexcept
        {
            return "\xE2\x99\x9C";
        }

        /** Returns a UTF8-encoded symbol for a Black queen. */
        [[nodiscard, gnu::const]] consteval std::string_view queen() noexcept
        {
            return "\xE2\x99\x9B";
        }

        /** Returns a UTF8-encoded symbol for a Black king. */
        [[nodiscard, gnu::const]] consteval std::string_view king() noexcept
        {
            return "\xE2\x99\x9A";
        }

        /// @}

    } // namespace black

    /// @ingroup pieces
    /// @{

    /** Returns a UTF8-encoded symbol for a pawn of the desired color. */
    [[nodiscard, gnu::const]] constexpr std::string_view pawn(const Color color) noexcept
    {
        if (color == Color::White)
            return white::pawn();

        return black::pawn();
    }

    /** Returns a UTF8-encoded symbol for a knight of the desired color. */
    [[nodiscard, gnu::const]] constexpr std::string_view knight(const Color color) noexcept
    {
        if (color == Color::White)
            return white::knight();

        return black::knight();
    }

    /** Returns a UTF8-encoded symbol for a bishop of the desired color. */
    [[nodiscard, gnu::const]] constexpr std::string_view bishop(const Color color) noexcept
    {
        if (color == Color::White)
            return white::bishop();

        return black::bishop();
    }

    /** Returns a UTF8-encoded symbol for a rook of the desired color. */
    [[nodiscard, gnu::const]] constexpr std::string_view rook(const Color color) noexcept
    {
        if (color == Color::White)
            return white::rook();

        return black::rook();
    }

    /** Returns a UTF8-encoded symbol for a queen of the desired color. */
    [[nodiscard, gnu::const]] constexpr std::string_view queen(const Color color) noexcept
    {
        if (color == Color::White)
            return white::queen();

        return black::queen();
    }

    /** Returns a UTF8-encoded symbol for a king of the desired color. */
    [[nodiscard, gnu::const]] constexpr std::string_view king(const Color color) noexcept
    {
        if (color == Color::White)
            return white::king();

        return black::king();
    }

    /// @}

} // namespace utf8

} // namespace chess::pieces
