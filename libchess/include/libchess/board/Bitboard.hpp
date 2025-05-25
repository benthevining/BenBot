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
#include <ranges>
#include <typeindex> // for std::hash

namespace chess::board {

using std::size_t;

/** This class is similar to ``std::bitset``, in that it is a simple collection of 64 bits,
    but it includes some convenience methods specific to usage as a bitboard.

    Each bit represents a square of the chessboard; the bit is 1 if there is a piece there,
    and 0 if the square is empty.

    Mapping ranks/files to indices of bits within a bitboard is handled by the Square class.

    @see Pieces, masks
    @ingroup board

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
    [[nodiscard]] constexpr size_t count() const noexcept { return bits.count(); }

    /** Returns true if there is a piece on the given square. */
    [[nodiscard]] constexpr bool test(const Square square) const noexcept { return test(square.index()); }

    /** Returns true if there is a piece on the given square.
        This method asserts if the given index is greater than 63.
     */
    [[nodiscard]] constexpr bool test(const BitboardIndex index) const noexcept
    {
        assert(index <= MAX_BITBOARD_IDX);
        return bits[index];
    }

    /** Sets the given square's bit to the given value. */
    constexpr void set(const Square square, const bool value = true) noexcept { set(square.index(), value); }

    /** Sets the given square's bit to the given value.
        This method asserts if the given index is greater than 63.
     */
    constexpr void set(const BitboardIndex index, const bool value = true) noexcept
    {
        assert(index <= MAX_BITBOARD_IDX);
        bits[index] = value;
    }

    /** Resets all bits to 0. */
    constexpr void clear() noexcept { bits.reset(); }

    /** Converts this bitboard to its integer representation. */
    [[nodiscard]] constexpr Integer to_int() const noexcept { return bits.to_ullong(); }

    /** Returns an iterable range of Square objects representing the 1 bits in this bitboard. */
    [[nodiscard]] constexpr auto squares() const noexcept
    {
        return std::views::iota(
                   0uz, static_cast<size_t>(NUM_SQUARES))
             | std::views::filter(
                 [this](const size_t index) { return test(static_cast<BitboardIndex>(index)); })
             | std::views::transform(
                 [](const size_t index) { return Square::from_index(static_cast<BitboardIndex>(index)); });
    }

private:
    std::bitset<NUM_SQUARES> bits;
};

/** This namespace provides some compile-time bitboard constants and masks.
    @ingroup board

    @todo center squares, perimeter squares
 */
namespace masks {

    /// @ingroup board
    /// @{

    /** Returns a bitboard with all bits set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard all() noexcept
    {
        return Bitboard { 0XFFFFFFFFFFFFFFFF };
    }

    /** Returns a bitboard with all the dark squares set to 1.
        @see light_squares()
     */
    [[nodiscard, gnu::const]] consteval Bitboard dark_squares() noexcept
    {
        return Bitboard { 0xAA55AA55AA55AA55 };
    }

    /** Returns a bitboard with all the light squares set to 1.
        @see dark_squares()
     */
    [[nodiscard, gnu::const]] consteval Bitboard light_squares() noexcept
    {
        return Bitboard { 0x55AA55AA55AA55AA };
    }

    /** Returns a bitboard with all squares on the A file set to 1.
        @see h_file()
     */
    [[nodiscard, gnu::const]] consteval Bitboard a_file() noexcept
    {
        return Bitboard { 0x0101010101010101 };
    }

    /** Returns a bitboard with all squares on the H file set to 1.
        @see a_file()
     */
    [[nodiscard, gnu::const]] consteval Bitboard h_file() noexcept
    {
        return Bitboard { 0x8080808080808080 };
    }

    /** Returns a bitboard with all squares on the first rank set to 1.
        @see rank_8()
     */
    [[nodiscard, gnu::const]] consteval Bitboard rank_1() noexcept
    {
        return Bitboard { 0x00000000000000FF };
    }

    /** Returns a bitboard with all squares on the eighth rank set to 1.
        @see rank_1()
     */
    [[nodiscard, gnu::const]] consteval Bitboard rank_8() noexcept
    {
        return Bitboard { 0xFF00000000000000 };
    }

    /** Returns a bitboard with all squares on the A1-H8 long diagonal set to 1.
        @see a8_h1_diagonal()
     */
    [[nodiscard, gnu::const]] consteval Bitboard a1_h8_diagonal() noexcept
    {
        return Bitboard { 0x8040201008040201 };
    }

    /** Returns a bitboard with all squares on the A1-H8 long diagonal set to 1.
        @see a1_h8_diagonal()
     */
    [[nodiscard, gnu::const]] consteval Bitboard a8_h1_diagonal() noexcept
    {
        return Bitboard { 0x0102040810204080 };
    }

    /// @}

    /** This namespace provides some compile-time bitboard constants for the starting positions
        of each piece type.

        @ingroup board
     */
    namespace starting {

        /** This namespace provides compile-time bitboard constants for the starting locations of
            the white pieces.

            @ingroup board
            @see black

            @todo pawns, rooks, knights, bishops, king, queen
         */
        namespace white {

            /// @ingroup board
            /// @{

            // TODO!!

            /// @}

        } // namespace white

        /** This namespace provides compile-time bitboard constants for the starting locations of
            the black pieces.

            @ingroup board
            @see white

            @todo pawns, rooks, knights, bishops, king, queen
         */
        namespace black {

            /// @ingroup board
            /// @{

            // TODO!!

            /// @}

        } // namespace black

    } // namespace starting

} // namespace masks

} // namespace chess::board

namespace std {

/** A hash specialization for Bitboard objects.
    Bitboards are hashed as their integer representations.

    @see chess::board::Bitboard
    @ingroup board
 */
template <>
struct hash<chess::board::Bitboard> final {
    [[nodiscard]] constexpr size_t operator()(const chess::board::Bitboard& board) const noexcept
    {
        return hash<chess::board::Bitboard::Integer> {}(board.to_int());
    }
};

} // namespace std
