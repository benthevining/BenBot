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

#include <charconv>
#include <concepts>
#include <string_view>
#include <utility>

namespace chess::util {

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
 */
[[nodiscard]] constexpr std::pair<std::string_view, std::string_view>
split_at_first_space(std::string_view input);

/** Reads an integer from the input string using ``std::from_chars``.
 */
template <std::integral Int>
[[nodiscard]] Int int_from_string(
    std::string_view text, Int defaultValue = 0) noexcept;

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

constexpr std::pair<std::string_view, std::string_view>
split_at_first_space(
    const std::string_view input)
{
    const auto spaceIdx = input.find(' ');

    if (spaceIdx == std::string_view::npos) {
        return { input, {} };
    }

    return {
        input.substr(0uz, spaceIdx),
        input.substr(spaceIdx + 1uz)
    };
}

template <std::integral Int>
[[nodiscard]] Int int_from_string(
    const std::string_view text, Int defaultValue) noexcept
{
    std::from_chars(text.data(), text.data() + text.length(), defaultValue);

    return defaultValue;
}

} // namespace chess::util
