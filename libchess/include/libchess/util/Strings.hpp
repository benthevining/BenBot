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

namespace chess::util {

/** Trims any whitespace characters from the beginning and ending
    of the string.

    @ingroup util
 */
[[nodiscard]] std::string_view trim(std::string_view text);

} // namespace chess::util
