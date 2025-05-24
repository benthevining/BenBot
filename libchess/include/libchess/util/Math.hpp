/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <concepts>

/** This namespace contains general maths utility functions used throughout libchess. */
namespace chess::math {

/** Returns true if the given integer value is an even number. */
template <std::integral T>
[[nodiscard, gnu::const]] constexpr bool is_even(T value) noexcept
{
    return (value & static_cast<T>(1)) == static_cast<T>(0);
}

} // namespace chess::math
