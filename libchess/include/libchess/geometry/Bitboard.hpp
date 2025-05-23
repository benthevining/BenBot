/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <bitset>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint>

namespace chess {

/** This class is similar to ``std::bitset``, in that it is a simple collection of 64 bits,
    but it includes some convenience methods specific to usage as a bitboard.

    Each bit represents a square of the chessboard; the bit is 1 if there is a piece there,
    and 0 if the square is empty.

    @todo test/set specific squares
 */
struct Bitboard final {
    /** Integer type used for serialization of bitboards. */
    using Integer = std::uint_least64_t;

    /** Constructs an empty bitboard. */
    constexpr Bitboard() noexcept = default;

    /** Constructs a bitboard from an integer representation. */
    explicit constexpr Bitboard(Integer value) noexcept
        : bits { static_cast<unsigned long long>(value) } // NOLINT(runtime/int)
    {
    }

    /** Returns true if the two bitboards have all the same bits set. */
    [[nodiscard]] constexpr bool operator==(const Bitboard& other) const noexcept
    {
        return bits == other.bits;
    }

    /** Returns true if any of the bits are set. */
    [[nodiscard]] constexpr bool any() const noexcept { return bits.any(); }

    /** Returns true if none of the bits are set. */
    [[nodiscard]] constexpr bool none() const noexcept { return bits.none(); }

    /** Returns the number of bits that are set. */
    [[nodiscard]] constexpr std::size_t count() const noexcept { return bits.count(); }

    /** Resets all bits to 0. */
    constexpr void reset() noexcept { bits.reset(); }

    /** Converts this bitboard to its integer representation. */
    [[nodiscard]] constexpr Integer to_int() const noexcept
    {
        return static_cast<Integer>(bits.to_ullong());
    }

private:
    std::bitset<64uz> bits;
};

} // namespace chess
