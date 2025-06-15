/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the Color enumeration type.
    @ingroup pieces
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_fast8_t

namespace chess::pieces {

/** This enum identifies the side to move.
    @ingroup pieces
 */
enum class Color : std::uint_fast8_t {
    Black, ///< The Black player.
    White  ///< The White player.
};

/** Returns the opposite color.

    @ingroup pieces
    @see Color
 */
template <Color Side>
[[nodiscard]] constexpr Color other_side() noexcept
{
    if constexpr (Side == Color::White) {
        return Color::Black;
    } else {
        return Color::White;
    }
}

} // namespace chess::pieces
