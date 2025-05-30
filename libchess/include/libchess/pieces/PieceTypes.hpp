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
#include <format>
#include <magic_enum/magic_enum.hpp>
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

/** This namespace contains constants encoding the material values of the various piece types.
    The king is not assigned a material value, as it can never be legally captured in a non-checkmated position.
    @ingroup pieces
 */
namespace values {

    /// @ingroup pieces
    /// @{

    /** The material value of a pawn (1). */
    static constexpr size_t PAWN { 1uz };

    /** The material value of a knight (3). */
    static constexpr size_t KNIGHT { 3uz };

    /** The material value of a bishop (4). */
    static constexpr size_t BISHOP { 4uz };

    /** The material value of a rook (5). */
    static constexpr size_t ROOK { 5uz };

    /** The material value of a queen (9). */
    static constexpr size_t QUEEN { 9uz };

    /// @}

} // namespace values

} // namespace chess::pieces

namespace std {

/** A formatter specialization for chess piece types.

    The formatter accepts the following format specifier arguments:
    @li ``s|S``: Tells the formatter to print a short (single-letter) version of the piece type
    @li ``l|L``: Tells the formatter to print a long version of the piece type

    If no arguments are specified, the formatter prints the short version of the piece type by default.

    @see chess::pieces::Type
    @ingroup pieces
 */
template <>
struct formatter<chess::pieces::Type> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx);

    template <typename FormatContext>
    typename FormatContext::iterator format(
        chess::pieces::Type piece, FormatContext& ctx) const;

private:
    bool useShort { true };
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

template <typename ParseContext>
constexpr typename ParseContext::iterator
formatter<chess::pieces::Type>::parse(ParseContext& ctx)
{
    auto it = ctx.begin();

    if (it == ctx.end() || *it == '}')
        return it;

    do {
        switch (*it) {
            case 's': [[fallthrough]];
            case 'S':
                useShort = true;
                break;

            case 'l': [[fallthrough]];
            case 'L':
                useShort = false;
                break;

            default:
                throw std::format_error { "Unrecognized format argument" };
        }

        ++it;
    } while (! (it == ctx.end() || *it == '}'));

    ctx.advance_to(it);

    return it;
}

template <typename FormatContext>
typename FormatContext::iterator
formatter<chess::pieces::Type>::format(
    const chess::pieces::Type piece, FormatContext& ctx) const
{
    if (useShort)
        return std::format_to(ctx.out(), "{}", chess::pieces::to_char(piece));

    return std::format_to(ctx.out(), "{}", magic_enum::enum_name(piece));
}

} // namespace std

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

constexpr Type from_string(const std::string_view text)
{
    if (text.length() == 1uz) {
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

    // TODO: handle case conversion

    if (const auto value = magic_enum::enum_cast<Type>(text))
        return *value;

    if (text == "Pawn")
        return Type::Pawn;

    throw std::invalid_argument {
        std::format("Cannot parse piece type from invalid input string: {}", text)
    };
}

} // namespace chess::pieces
