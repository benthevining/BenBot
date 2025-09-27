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

/** @file
    This file defines the File enumeration.
    @ingroup board
 */

#pragma once

#include <cctype> // IWYU pragma: keep - for std::tolower()
#include <expected>
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <magic_enum/magic_enum.hpp>
#include <string>

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

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup board
    @see File
 */
[[nodiscard]] auto file_from_char(char character)
    -> std::expected<File, std::string>;

/** Converts the file enumeration to its single-character representation.

    @ingroup board
    @see File
 */
[[nodiscard]] auto file_to_char(File file) -> char;

} // namespace chess::board

/** A formatter specialization for chessboard files.
    The formatter accepts no arguments; files are always printed as lowercase letters.

    @see chess::board::File
    @ingroup board
 */
template <>
struct std::formatter<chess::board::File> final {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(
        chess::board::File file, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(), "{}", chess::board::file_to_char(file));
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

namespace chess::board {

inline auto file_from_char(char character)
    -> std::expected<File, std::string>
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
            return std::unexpected(
                std::format(
                    "Cannot parse File from character: {}",
                    character));
    }
}

inline auto file_to_char(const File file) -> char
{
    const auto upperChar = magic_enum::enum_name(file).front();

    return static_cast<char>(
        std::tolower(static_cast<unsigned char>(upperChar)));
}

} // namespace chess::board
