/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup util Utilities
    Generic utilities used throughout libchess.
 */

/** @file
    This file provides some general maths utility functions.
    @ingroup util
 */

#pragma once

#include <concepts>

/** This namespace contains general maths utility functions used throughout libchess.
    @ingroup util
 */
namespace chess::math {

/// @ingroup util
/// @{

/** Returns true if the given integer value is divisible by 2. */
template <std::integral Int>
[[nodiscard, gnu::const]] constexpr bool is_even(Int value) noexcept
{
    return (value & static_cast<Int>(1)) == static_cast<Int>(0);
}

/// @}

} // namespace chess::math
