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
#include <format>
#include <stdexcept>
#include <string_view>
#include <utility>

/** This namespace contains classes for encoding information about the various chess piece types.
    @ingroup pieces
 */
namespace chess::pieces {

using std::size_t;

/** This enum identifies the different types of pieces.

    @ingroup pieces
    @see utf8 values
 */
enum class Type : std::uint_fast8_t {
    Pawn,   ///< A White pawn.
    Knight, ///< A knight.
    Bishop, ///< A bishop.
    Rook,   ///< A rook.
    Queen,  ///< A queen.
    King    ///< A king.
};

/** Parses a piece type from a string.
    This function recognizes single-letter abbreviations (such as ``N`` for knight, etc.),
    or full piece names.

    @throws std::invalid_argument An exception will be thrown if the input string cannot
    be parsed correctly.

    @ingroup pieces
 */
[[nodiscard, gnu::const]] constexpr Type from_string(std::string_view text);

/** Converts the given piece type to its single-character representation.

    @ingroup pieces
 */
[[nodiscard, gnu::const]] constexpr char to_char(Type type, bool uppercase = true) noexcept;

} // namespace chess::pieces

/** A formatter specialization for chess piece types.
    The formatter accepts no arguments. Piece types are printed as a single upper-case letter.

    @see chess::pieces::Type
    @ingroup pieces
 */
template <>
struct std::formatter<chess::pieces::Type> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(
        chess::pieces::Type piece, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", chess::pieces::to_char(piece));
    }
};

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

namespace chess::pieces {

constexpr char to_char(const Type type, const bool uppercase) noexcept
{
    if (uppercase) {
        static constexpr std::string_view upperChars { "PNBRQK" };

        return upperChars[std::to_underlying(type)];
    }

    static constexpr std::string_view lowerChars { "pnbrqk" };

    return lowerChars[std::to_underlying(type)];
}

constexpr Type from_string(std::string_view text)
{
    if (text.length() != 1uz)
        text = text.substr(0uz, 1uz);

    switch (text.front()) {
        case 'p': [[fallthrough]];
        case 'P': return Type::Pawn;

        case 'n': [[fallthrough]];
        case 'N': return Type::Knight;

        case 'b': [[fallthrough]];
        case 'B': return Type::Bishop;

        case 'r': [[fallthrough]];
        case 'R': return Type::Rook;

        case 'q': [[fallthrough]];
        case 'Q': return Type::Queen;

        case 'k': [[fallthrough]];
        case 'K': return Type::King;

        default:
            throw std::invalid_argument {
                std::format("Cannot parse piece type from invalid input string: {}", text)
            };
    }
}

} // namespace chess::pieces
