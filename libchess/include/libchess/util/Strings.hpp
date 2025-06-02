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

#include <string_view>
#include <utility>

namespace chess::util {

/** Trims any whitespace characters from the beginning and ending
    of the string.

    @ingroup util
 */
[[nodiscard]] std::string_view trim(std::string_view text);

/** Splits the input string into segments before and after the first
    whitespace character. If there is no whitespace in the input
    string, the returned pair contains a copy of the input string
    and an empty string_view.

    @ingroup util
 */
[[nodiscard]] constexpr std::pair<std::string_view, std::string_view>
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

} // namespace chess::util
