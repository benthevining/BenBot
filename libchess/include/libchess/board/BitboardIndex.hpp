/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the BitboardIndex type.
    @ingroup board
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_fast8_t

namespace chess::board {

/** Unsigned integer type used for bitboard indices.
    Valid bitboard indices are in the range ``[0, 63]``.

    @ingroup board
    @see NUM_SQUARES, MAX_BITBOARD_IDX
 */
using BitboardIndex = std::uint_fast8_t;

/** The number of squares on a chessboard.

    @ingroup board
    @see MAX_BITBOARD_IDX
 */
static constexpr auto NUM_SQUARES = static_cast<BitboardIndex>(64);

/** The maximum valid bitboard bit index.

    @ingroup board
    @see NUM_SQUARES
 */
static constexpr auto MAX_BITBOARD_IDX = NUM_SQUARES - static_cast<BitboardIndex>(1);

} // namespace chess::board
