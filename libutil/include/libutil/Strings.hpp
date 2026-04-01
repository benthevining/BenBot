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

/** @defgroup strings String utilities
    String handling utility functions used throughout the code.

    @ingroup util
 */

/** @file
    This file provides some string handling utility functions.
    @ingroup strings
 */

#pragma once

#include <array>
#include <charconv>
#include <concepts>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <expected>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

/** This namespace string handling functions.
    @ingroup strings
 */
namespace util::strings {

using std::ptrdiff_t;
using std::size_t;
using std::string_view;

/** Utility typedef for a pair of string views.
    @ingroup strings
 */
using StringViewPair = std::pair<string_view, string_view>;

/// @ingroup strings
/// @{

/** Trims any whitespace characters from the beginning and ending
    of the string, including space and newline characters.
 */
[[nodiscard]] auto trim(string_view text) -> string_view;

/** Splits the input string into segments before and after the first
    whitespace character. If there is no whitespace in the input
    string, the returned pair contains a copy of the input string
    and an empty string_view. If the first character of the input
    string is a space, the first element of the returned pair is
    an empty string_view.

    @see split_at_first_space_or_newline()
 */
[[nodiscard]] auto split_at_first_space(string_view input) -> StringViewPair;

/** Similar to ``split_at_first_space()``, but also splits on newlines.

    @see split_at_first_space()
 */
[[nodiscard]] auto split_at_first_space_or_newline(string_view input) -> StringViewPair;

/** For a string beginning with ``(``, finds the index of the matching ``)``
    character, taking nested ``()`` pairs into account. This function asserts
    if the ``input`` does not begin with ``(``.

    If a matching close parentheses character isn't found, returns an explanatory
    error string.
 */
[[nodiscard]] auto find_matching_close_paren(string_view input)
    -> std::expected<size_t, std::string>;

/** Reads an integer from the input string using ``std::from_chars``.

    @see write_integer()
 */
template <std::integral Int>
[[nodiscard]] constexpr auto int_from_string(
    string_view text, Int defaultValue = 0) noexcept
    -> Int;

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

/** Returns a range of string_views created by splitting the input
    text at the given delimiter.

    @see lines_view(), words_view()
 */
[[nodiscard]] auto split_by_delim(string_view text, char delim);

/** Returns a range of string_views, each representing a line from the
    input string.

    @see words_view(), split_by_delim()
 */
[[nodiscard]] auto lines_view(string_view text);

/** Returns a range of string_views, each representing a word from the
    input string.

    @see lines_view(), split_by_delim()
 */
[[nodiscard]] auto words_view(string_view text);

/** Computes the Levenshtein distance between the two strings. */
[[nodiscard, gnu::const]] auto levenshtein_distance(
    string_view first, string_view second) -> size_t;

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
[[nodiscard]] constexpr Int int_from_string(
    const string_view text, Int defaultValue) noexcept
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
    std::array<char, MaxLen + 1uz> buffer { };

    const auto result = std::to_chars(
        buffer.data(),
        std::next(buffer.data(), static_cast<ptrdiff_t>(buffer.size())),
        value);

    // simply do nothing on failure
    if (result.ec != std::errc { })
        return;

    output.append(
        buffer.data(),
        static_cast<size_t>(std::distance(buffer.data(), result.ptr)));
}

inline auto split_by_delim(string_view text, char delim)
{
    return text
         | std::views::split(delim)
         | std::views::transform([](const auto rng) { return string_view { rng }; });
}

inline auto lines_view(const string_view text)
{
    return split_by_delim(text, '\n');
}

inline auto words_view(const string_view text)
{
    return split_by_delim(text, ' ');
}

} // namespace util::strings
