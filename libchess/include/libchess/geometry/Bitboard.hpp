/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <libchess/geometry/Square.hpp>

namespace chess {

/** This class is similar to ``std::bitset``, in that it is a simple collection of 64 bits,
    but it includes some convenience methods specific to usage as a bitboard.

    Each bit represents a square of the chessboard; the bit is 1 if there is a piece there,
    and 0 if the square is empty.
 */
struct Bitboard final {
    /** Unsigned integer type used for serialization of bitboards. */
    using Integer = std::uint_least64_t;

    /** Constructs an empty bitboard. */
    constexpr Bitboard() noexcept = default;

    /** Constructs a bitboard from an integer representation.
        @see to_int()
     */
    explicit constexpr Bitboard(Integer value) noexcept
        : bits { static_cast<unsigned long long>(value) } // NOLINT(runtime/int)
    {
    }

    /** Returns true if the two bitboards have all the same bits set. */
    [[nodiscard]] constexpr bool operator==(const Bitboard& other) const noexcept { return bits == other.bits; }

    /** Returns true if any of the bits are set. */
    [[nodiscard]] constexpr bool any() const noexcept { return bits.any(); }

    /** Returns true if none of the bits are set. */
    [[nodiscard]] constexpr bool none() const noexcept { return bits.none(); }

    /** Returns the number of bits that are set. */
    [[nodiscard]] constexpr std::size_t count() const noexcept { return bits.count(); }

    /** Returns true if there is a piece on the given square. */
    [[nodiscard]] constexpr bool test(Square square) const noexcept { return test(square.index()); }

    /** Returns true if there is a piece on the given square.
        This method asserts if the given index is greater than 63.
     */
    [[nodiscard]] constexpr bool test(BitboardIndex index) const noexcept
    {
        assert(index <= 63uz);
        return bits[index];
    }

    /** Sets the given square's bit to the given value. */
    constexpr void set(Square square, bool value = true) noexcept { set(square.index(), value); }

    /** Sets the given square's bit to the given value.
        This method asserts if the given index is greater than 63.
     */
    constexpr void set(BitboardIndex index, bool value = true) noexcept
    {
        assert(index <= 63uz);
        bits[index] = value;
    }

    /** Resets all bits to 0. */
    constexpr void clear() noexcept { bits.reset(); }

    /** Converts this bitboard to its integer representation. */
    [[nodiscard]] constexpr Integer to_int() const noexcept { return static_cast<Integer>(bits.to_ullong()); }

private:
    std::bitset<64uz> bits;
};

} // namespace chess
