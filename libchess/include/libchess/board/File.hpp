/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the File enumeration.
    @ingroup board
 */

#pragma once

#include <cctype> // IWYU pragma: keep - for std::tolower()
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>

namespace chess::board {

/** This enum describes the files of the chess board.

    @see Rank
    @ingroup board
 */
enum class File : BitboardIndex {
    A, ///< The A file.
    B, ///< The B file.
    C, ///< The C file.
    D, ///< The D file. This is the file that the queens start on.
    E, ///< The E file. This is the file that the kings start on.
    F, ///< The F file.
    G, ///< The G file.
    H  ///< The H file.
};

/** Interprets the given character as a file.
    This function recognizes upper- or lowercase file letters.

    @throws std::invalid_argument An exception will be thrown if a file
    cannot be parsed correctly from the input character.
 */
[[nodiscard, gnu::const]] constexpr File file_from_char(char character);

} // namespace chess::board

namespace std {

/** A formatter specialization for chessboard files.

    The formatter accepts the following format specifier arguments:
    @li ``u|U``: Tells the formatter to print the file as an uppercase letter
    @li ``l|L``: Tells the formatter to print the file as a lowercase letter

    If no arguments are specified, the formatter prints the rank as a lowercase letter by default.

    @see chess::board::File
    @ingroup board
 */
template <>
struct formatter<chess::board::File> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx);

    template <typename FormatContext>
    typename FormatContext::iterator format(
        chess::board::File file, FormatContext& ctx) const;

private:
    bool uppercase { false };
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
formatter<chess::board::File>::parse(ParseContext& ctx)
{
    auto it = ctx.begin();

    if (it == ctx.end() || *it == '}')
        return it;

    do {
        switch (*it) {
            case 'u': [[fallthrough]];
            case 'U':
                uppercase = true;
                break;

            case 'l': [[fallthrough]];
            case 'L':
                uppercase = false;
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
formatter<chess::board::File>::format(
    const chess::board::File file, FormatContext& ctx) const
{
    const auto character = [file, upper = uppercase] {
        const auto upperChar = magic_enum::enum_name(file).front();

        if (! upper)
            return static_cast<char>(
                std::tolower(static_cast<unsigned char>(upperChar)));

        return upperChar;
    }();

    return std::format_to(ctx.out(), "{}", character);
}

} // namespace std

namespace chess::board {

constexpr File file_from_char(char character)
{
    switch (character) {
        case 'a': [[fallthrough]];
        case 'A': return File::A;

        case 'b': [[fallthrough]];
        case 'B': return File::B;

        case 'c': [[fallthrough]];
        case 'C': return File::C;

        case 'd': [[fallthrough]];
        case 'D': return File::D;

        case 'e': [[fallthrough]];
        case 'E': return File::E;

        case 'f': [[fallthrough]];
        case 'F': return File::F;

        case 'g': [[fallthrough]];
        case 'G': return File::G;

        case 'h': [[fallthrough]];
        case 'H': return File::H;

        default:
            throw std::invalid_argument {
                std::format("Cannot parse File from character: {}", character)
            };
    }
}

} // namespace chess::board
