/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the Bitboard class, and some compile-time bitboard constants & masks.
    @ingroup board
 */

#pragma once

#include <bitset>
#include <cassert>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint_least64_t
#include <libchess/board/Square.hpp>

namespace chess::board {

/** This class is similar to ``std::bitset``, in that it is a simple collection of 64 bits,
    but it includes some convenience methods specific to usage as a bitboard.

    Each bit represents a square of the chessboard; the bit is 1 if there is a piece there,
    and 0 if the square is empty.

    Mapping ranks/files to indices of bits within a bitboard is handled by the Square class.

    @see Pieces masks
    @ingroup board

    @todo Masks for starting positions of each piece type
    @todo std::formatter
 */
struct Bitboard final {
    /** Unsigned integer type used for serialization of bitboards. */
    using Integer = std::uint_least64_t;

    /** Constructs an empty bitboard. */
    constexpr Bitboard() noexcept = default;

    /** Constructs a bitboard from an integer representation.
        @see to_int()
     */
    explicit constexpr Bitboard(const Integer value) noexcept
        : bits { static_cast<unsigned long long>(value) } // NOLINT
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
    [[nodiscard]] constexpr bool test(const Square square) const noexcept { return test(square.index()); }

    /** Returns true if there is a piece on the given square.
        This method asserts if the given index is greater than 63.
     */
    [[nodiscard]] constexpr bool test(const BitboardIndex index) const noexcept
    {
        assert(index <= 63uz);
        return bits[index];
    }

    /** Sets the given square's bit to the given value. */
    constexpr void set(const Square square, const bool value = true) noexcept { set(square.index(), value); }

    /** Sets the given square's bit to the given value.
        This method asserts if the given index is greater than 63.
     */
    constexpr void set(const BitboardIndex index, const bool value = true) noexcept
    {
        assert(index <= 63uz);
        bits[index] = value;
    }

    /** Resets all bits to 0. */
    constexpr void clear() noexcept { bits.reset(); }

    /** Converts this bitboard to its integer representation. */
    [[nodiscard]] constexpr Integer to_int() const noexcept { return bits.to_ullong(); }

private:
    std::bitset<64uz> bits;
};

/** This namespace provides some compile-time bitboard constants and masks.
    @ingroup board
 */
namespace masks {

    /// @ingroup board
    /// @{

    /** Returns a bitboard with all the dark squares set to 1.
        @see light_squares()
     */
    [[nodiscard]] consteval Bitboard dark_squares() noexcept
    {
        return Bitboard { 0xAA55AA55AA55AA55 };
    }

    /** Returns a bitboard with all the light squares set to 1.
        @see dark_squares()
     */
    [[nodiscard]] consteval Bitboard light_squares() noexcept
    {
        return Bitboard { 0x55AA55AA55AA55AA };
    }

    /** Returns a bitboard with all squares on the A file set to 1.
        @see h_file()
     */
    [[nodiscard]] consteval Bitboard a_file() noexcept
    {
        return Bitboard { 0x0101010101010101 };
    }

    /** Returns a bitboard with all squares on the H file set to 1.
        @see a_file()
     */
    [[nodiscard]] consteval Bitboard h_file() noexcept
    {
        return Bitboard { 0x8080808080808080 };
    }

    /** Returns a bitboard with all squares on the first rank set to 1.
        @see rank_8()
     */
    [[nodiscard]] consteval Bitboard rank_1() noexcept
    {
        return Bitboard { 0x00000000000000FF };
    }

    /** Returns a bitboard with all squares on the eighth rank set to 1.
        @see rank_1()
     */
    [[nodiscard]] consteval Bitboard rank_8() noexcept
    {
        return Bitboard { 0xFF00000000000000 };
    }

    /** Returns a bitboard with all squares on the A1-H8 long diagonal set to 1.
        @see a8_h1_diagonal()
     */
    [[nodiscard]] consteval Bitboard a1_h8_diagonal() noexcept
    {
        return Bitboard { 0x8040201008040201 };
    }

    /** Returns a bitboard with all squares on the A1-H8 long diagonal set to 1.
        @see a1_h8_diagonal()
     */
    [[nodiscard]] consteval Bitboard a8_h1_diagonal() noexcept
    {
        return Bitboard { 0x0102040810204080 };
    }

    /// @}

} // namespace masks

} // namespace chess::board
