/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides some string handling utility functions.
    @ingroup util
 */

#pragma once

#include <array>
#include <charconv>
#include <concepts>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <iterator>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace chess::util {

using std::ptrdiff_t;
using std::size_t;

using StringViewPair = std::pair<std::string_view, std::string_view>;

/// @ingroup util
/// @{

/** Trims any whitespace characters from the beginning and ending
    of the string.
 */
[[nodiscard]] std::string_view trim(std::string_view text);

/** Splits the input string into segments before and after the first
    whitespace character. If there is no whitespace in the input
    string, the returned pair contains a copy of the input string
    and an empty string_view. If the first character of the input
    string is a space, the first element of the returned pair is
    an empty string_view.

    @see split_at_first_space_or_newline()
 */
[[nodiscard]] StringViewPair split_at_first_space(std::string_view input);

/** Similar to ``split_at_first_space()``, but also splits on newlines.

    @see split_at_first_space()
 */
[[nodiscard]] StringViewPair split_at_first_space_or_newline(std::string_view input);

/** For a string beginning with ``(``, finds the index of the matching ``)``
    character, taking nested ``()`` pairs into account. This function asserts
    if the ``input`` does not begin with ``(``.

    @throws std::invalid_argument An exception will be thrown if no matching
    ``)`` character is found.
 */
[[nodiscard]] size_t find_matching_close_paren(std::string_view input);

/** Reads an integer from the input string using ``std::from_chars``.

    @see write_integer()
 */
template <std::integral Int>
[[nodiscard]] Int int_from_string(
    std::string_view text, Int defaultValue = 0) noexcept;

/** Appends an integer to the output string using ``std::to_chars``.
    This function uses stack memory for ``to_chars()`` to write into.
    If ``to_chars()`` returns an error, ``output`` is not changed.

    @tparam MaxLen The maximum number of characters that will be written
    to ``output``.

    @see int_from_string()
 */
template <size_t MaxLen = 5uz>
void write_integer(
    std::integral auto value,
    std::string&       output);

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

template <std::integral Int>
[[nodiscard]] Int int_from_string(
    const std::string_view text, Int defaultValue) noexcept
{
    std::from_chars(
        text.data(),
        std::next(text.data(), static_cast<ptrdiff_t>(text.length())),
        defaultValue);

    return defaultValue;
}

template <size_t MaxLen>
void write_integer(
    const std::integral auto value,
    std::string&             output)
{
    std::array<char, MaxLen> buffer {};

    const auto result = std::to_chars(
        buffer.data(),
        std::next(buffer.data(), static_cast<ptrdiff_t>(buffer.size())),
        value);

    if (result.ec != std::errc {})
        return;

    output.append(
        buffer.data(),
        static_cast<size_t>(std::distance(buffer.data(), result.ptr)));
}

} // namespace chess::util
