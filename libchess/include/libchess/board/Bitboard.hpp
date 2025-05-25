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
#include <format>
#include <libchess/board/Square.hpp>
#include <ranges>
#include <string>
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
    [[nodiscard]] constexpr bool operator==(const Bitboard& other) const noexcept = default;

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

    /** Returns an iterable list of indices representing the 1 bits in this bitboard.
        The returned indices should be iterated by value, not by reference; i.e.:
        @code{.cpp}
        for (auto index : board.indices())
          ; // ...
        @endcode
        @see squares()
     */
    [[nodiscard]] constexpr auto indices() const noexcept;

    /** Returns an iterable range of Square objects representing the 1 bits in this bitboard.
        The Square objects should be iterated by value, not by reference; i.e.:
        @code{.cpp}
        for (auto square : board.squares())
          ; // ...
        @endcode
        @see indices()
     */
    [[nodiscard]] constexpr auto squares() const noexcept;

    /** Performs binary AND with another bitboard. */
    constexpr Bitboard& operator&=(const Bitboard& other) noexcept
    {
        bits &= other.bits;
        return *this;
    }

    /** Performs binary OR with another bitboard. */
    constexpr Bitboard& operator|=(const Bitboard& other) noexcept
    {
        bits |= other.bits;
        return *this;
    }

    /** Performs binary XOR with another bitboard. */
    constexpr Bitboard& operator^=(const Bitboard& other) noexcept
    {
        bits ^= other.bits;
        return *this;
    }

    /** Returns a copy of this bitboard with all bits flipped (binary NOT). */
    [[nodiscard]] constexpr Bitboard inverse() const noexcept
    {
        auto copy = *this;
        copy.bits.flip();
        return copy;
    }

private:
    std::bitset<NUM_SQUARES> bits;
};

/** Creates an ASCII representation of the given bitboard.
    The returned string is meant to be interpreted visually by a human, probably for debugging purposes.
    The bitboard is drawn as a simple set of cells separated by ``|`` characters. Occupied squares have an
    ``x`` placed in them.

    @ingroup board
    @relates Bitboard
 */
[[nodiscard]] std::string print_ascii(const Bitboard& board);

} // namespace chess::board

namespace std {

/** A formatter specialization for Bitboard objects.

    The formatter accepts the following format specifier arguments:
    @li ``i|I``: Tells the formatter to print the bitboard as its integer representation. The integer is displayed in hexadecimal.
    @li ``g|G``: Tells the formatter to print a graphical representation of the bitboard.

    If no arguments are specified, the formatter prints the bitboard's integer representation by default.

    @see chess::board::Bitboard
    @ingroup board
 */
template <>
struct formatter<chess::board::Bitboard> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx);

    template <typename FormatContext>
    typename FormatContext::iterator format(
        const chess::board::Bitboard& board, FormatContext& ctx) const;

private:
    bool asInt { true };
};

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

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

template <typename ParseContext>
constexpr typename ParseContext::iterator
formatter<chess::board::Bitboard>::parse(ParseContext& ctx)
{
    auto it = ctx.begin();

    if (it == ctx.end() || *it == '}')
        return it;

    do {
        switch (*it) {
            case 'i': [[fallthrough]];
            case 'I':
                asInt = true;
                break;

            case 'g': [[fallthrough]];
            case 'G':
                asInt = false;
                break;

            default:
                throw std::format_error { "Unrecognized format argument" };
        }

        ++it;
    } while (! (it == ctx.end() || *it == '}'));

    ctx.advance_to(it);

    return it;
}

template <typename FormatContext>
typename FormatContext::iterator
formatter<chess::board::Bitboard>::format(
    const chess::board::Bitboard& board, FormatContext& ctx) const
{
    if (asInt)
        return std::format_to(ctx.out(), "{:#X}", board.to_int());

    return std::format_to(ctx.out(), "{}", print_ascii(board));
}

} // namespace std

namespace chess::board {

constexpr auto Bitboard::indices() const noexcept
{
    return std::views::iota(
               0uz, static_cast<size_t>(NUM_SQUARES))
         | std::views::filter(
             [this](const size_t index) { return test(static_cast<BitboardIndex>(index)); });
}

constexpr auto Bitboard::squares() const noexcept
{
    return indices()
         | std::views::transform(
             [](const size_t index) { return Square::from_index(static_cast<BitboardIndex>(index)); });
}

} // namespace chess::board
