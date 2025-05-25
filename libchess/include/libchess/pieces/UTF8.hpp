/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup utf8_pieces UTF8 encoded piece symbols
    This group provides UTF8 encoded piece symbols.
    @ingroup pieces
 */

/** @file
    This file provides UTF8 encoded piece symbols.
    @ingroup utf8_pieces
 */

#pragma once

#include <libchess/pieces/Colors.hpp>
#include <string_view>

/** This namespace contains some UTF8 encodings of chess piece symbols.
    @ingroup utf8_pieces
 */
namespace chess::pieces::utf8 {

/** This namespace contains some UTF8 encoding of White chess piece symbols.

    @ingroup utf8_pieces
    @see black
 */
namespace white {

    /// @ingroup utf8_pieces
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

    @ingroup utf8_pieces
    @see white
 */
namespace black {

    /// @ingroup utf8_pieces
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

/// @ingroup utf8_pieces
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

} // namespace chess::pieces::utf8
