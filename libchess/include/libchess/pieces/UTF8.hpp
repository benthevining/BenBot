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
#include <libchess/pieces/PieceTypes.hpp>
#include <string_view>
#include <utility> // IWYU pragma: keep - for std::unreachable()

/** This namespace contains some UTF8 encodings of chess piece symbols.
    @ingroup utf8_pieces
 */
namespace chess::pieces::utf8 {

using std::string_view;

/** This namespace contains some UTF8 encoding of White chess piece symbols.

    @ingroup utf8_pieces
    @see black
 */
namespace white {

    /// @ingroup utf8_pieces
    /// @{

    /** A UTF8-encoded symbol for a White pawn. */
    inline constexpr string_view PAWN { "\xE2\x99\x99" };

    /** A UTF8-encoded symbol for a White knight. */
    inline constexpr string_view KNIGHT { "\xE2\x99\x98" };

    /** A UTF8-encoded symbol for a White bishop. */
    inline constexpr string_view BISHOP { "\xE2\x99\x97" };

    /** A UTF8-encoded symbol for a White rook. */
    inline constexpr string_view ROOK { "\xE2\x99\x96" };

    /** A UTF8-encoded symbol for a White queen. */
    inline constexpr string_view QUEEN { "\xE2\x99\x95" };

    /** A UTF8-encoded symbol for a White king. */
    inline constexpr string_view KING { "\xE2\x99\x94" };

    /** Returns a UTF8-encoded symbol for a White piece of the given type. */
    [[nodiscard, gnu::const]] constexpr auto get(const Type type) noexcept -> string_view
    {
        switch (type) {
            case Type::Knight: return KNIGHT;
            case Type::Bishop: return BISHOP;
            case Type::Rook  : return ROOK;
            case Type::Queen : return QUEEN;
            case Type::King  : return KING;
            case Type::Pawn  : return PAWN;
            default          : std::unreachable();
        }
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

    /** A UTF8-encoded symbol for a Black pawn. */
    inline constexpr string_view PAWN { "\xE2\x99\x9F" };

    /** A UTF8-encoded symbol for a Black knight. */
    inline constexpr string_view KNIGHT { "\xE2\x99\x9E" };

    /** A UTF8-encoded symbol for a Black bishop. */
    inline constexpr string_view BISHOP { "\xE2\x99\x9D" };

    /** A UTF8-encoded symbol for a Black rook. */
    inline constexpr string_view ROOK { "\xE2\x99\x9C" };

    /** A UTF8-encoded symbol for a Black queen. */
    inline constexpr string_view QUEEN { "\xE2\x99\x9B" };

    /** A UTF8-encoded symbol for a Black king. */
    inline constexpr string_view KING { "\xE2\x99\x9A" };

    /** Returns a UTF8-encoded symbol for a Black piece of the given type. */
    [[nodiscard, gnu::const]] constexpr auto get(const Type type) noexcept -> string_view
    {
        switch (type) {
            case Type::Knight: return KNIGHT;
            case Type::Bishop: return BISHOP;
            case Type::Rook  : return ROOK;
            case Type::Queen : return QUEEN;
            case Type::King  : return KING;
            case Type::Pawn  : return PAWN;
            default          : std::unreachable();
        }
    }

    /// @}

} // namespace black

/// @ingroup utf8_pieces
/// @{

/** Returns a UTF8-encoded symbol for a pawn of the desired color. */
[[nodiscard, gnu::const]] constexpr auto pawn(const Color color) noexcept -> string_view
{
    if (color == Color::White)
        return white::PAWN;

    return black::PAWN;
}

/** Returns a UTF8-encoded symbol for a knight of the desired color. */
[[nodiscard, gnu::const]] constexpr auto knight(const Color color) noexcept -> string_view
{
    if (color == Color::White)
        return white::KNIGHT;

    return black::KNIGHT;
}

/** Returns a UTF8-encoded symbol for a bishop of the desired color. */
[[nodiscard, gnu::const]] constexpr auto bishop(const Color color) noexcept -> string_view
{
    if (color == Color::White)
        return white::BISHOP;

    return black::BISHOP;
}

/** Returns a UTF8-encoded symbol for a rook of the desired color. */
[[nodiscard, gnu::const]] constexpr auto rook(const Color color) noexcept -> string_view
{
    if (color == Color::White)
        return white::ROOK;

    return black::ROOK;
}

/** Returns a UTF8-encoded symbol for a queen of the desired color. */
[[nodiscard, gnu::const]] constexpr auto queen(const Color color) noexcept -> string_view
{
    if (color == Color::White)
        return white::QUEEN;

    return black::QUEEN;
}

/** Returns a UTF8-encoded symbol for a king of the desired color. */
[[nodiscard, gnu::const]] constexpr auto king(const Color color) noexcept -> string_view
{
    if (color == Color::White)
        return white::KING;

    return black::KING;
}

/// @}

} // namespace chess::pieces::utf8
