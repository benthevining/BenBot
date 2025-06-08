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

#include <bit>
#include <cassert>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <format>
#include <iterator>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Square.hpp>
#include <ranges>
#include <string>

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
    /** Unsigned integer type used for bitboard representation. */
    using Integer = std::uint64_t;

    /** Constructs an empty bitboard. */
    constexpr Bitboard() noexcept = default;

    /** Constructs a bitboard from an integer representation.
        @see to_int()
     */
    explicit constexpr Bitboard(Integer val) noexcept;

    /** Returns true if the two bitboards have all the same bits set. */
    [[nodiscard]] constexpr bool operator==(const Bitboard&) const noexcept = default;

    /// @name Observers
    /// @{

    /** Returns true if any of the bits are set. */
    [[nodiscard]] constexpr bool any() const noexcept { return value != 0; }

    /** Returns true if none of the bits are set. */
    [[nodiscard]] constexpr bool none() const noexcept { return value == 0; }

    /** Returns the number of bits that are set. */
    [[nodiscard]] constexpr size_t count() const noexcept { return std::popcount(value); }

    /** Returns true if there is a piece on the given square. */
    [[nodiscard]] constexpr bool test(const Square square) const noexcept { return test(square.index()); }

    /** Returns true if there is a piece on the given square.
        This method asserts if the given index is greater than 63.
     */
    [[nodiscard]] constexpr bool test(BitboardIndex index) const noexcept;

    /** Returns the index of the first set bit.
        This operation may also be known as "bitscan forward".
        Returns 64 if all bits are 0.
     */
    [[nodiscard]] constexpr BitboardIndex first() const noexcept;

    /** Returns the index of the last set bit.
        This operation may also be known as "bitscan reverse".
        Returns 64 if all bits are 0.
     */
    [[nodiscard]] constexpr BitboardIndex last() const noexcept;

    /// @}

    /// @name Modifiers
    /// @{

    /** Sets the given square's bit to 1. */
    constexpr void set(const Square square) noexcept { set(square.index()); }

    /** Sets the given square's bit to 1.
        This method asserts if the given index is greater than 63.
     */
    constexpr void set(BitboardIndex index) noexcept;

    /** Sets the given square's bit to 0. */
    constexpr void unset(const Square square) noexcept { unset(square.index()); }

    /** Sets the given square's bit to 0.
        This method asserts if the given index is greater than 63.
     */
    constexpr void unset(BitboardIndex index) noexcept;

    /** Resets all bits to 0. */
    constexpr void clear() noexcept { value = 0; }

    /// @}

    /** Converts this bitboard to its integer representation. */
    [[nodiscard]] constexpr Integer to_int() const noexcept { return value; }

    /// @name Iteration
    /// @{

    /** Returns an iterable range of indices representing the 1 bits in this bitboard.
        The returned indices should be iterated by value, not by reference; i.e.:
        @code{.cpp}
        for (auto index : board.indices())
          ; // ...
        @endcode
        @see squares(), subboards()
     */
    [[nodiscard]] constexpr auto indices() const noexcept;

    /** Returns an iterable range of Square objects representing the 1 bits in this bitboard.
        The Square objects should be iterated by value, not by reference; i.e.:
        @code{.cpp}
        for (auto square : board.squares())
          ; // ...
        @endcode
        @see indices(), subboards()
     */
    [[nodiscard]] constexpr auto squares() const noexcept;

    /** Returns an iterable range of Bitboard objects that each have a single bit set, each
        representing the 1 bits in this bitboard. This is a transformation of a single bitboard
        with up to 64 bits set into a set of up to 64 bitboards each with a single bit set.

        The Bitboard objects should be iterated by value, not by reference; i.e.:
        @code{.cpp}
        for (auto subboard : board.subboards())
          ; // ...
        @endcode
        @see indices(), squares()
     */
    [[nodiscard]] constexpr auto subboards() const noexcept;

    /// @}

    /// @name Binary operations
    /// @{

    /** Returns a copy of this bitboard with all bits flipped (binary NOT). */
    [[nodiscard]] constexpr Bitboard inverse() const noexcept;

    /** Performs binary AND with another bitboard. */
    constexpr Bitboard& operator&=(const Bitboard& other) noexcept;

    /** Performs binary OR with another bitboard. */
    constexpr Bitboard& operator|=(const Bitboard& other) noexcept;

    /** Performs binary XOR with another bitboard. */
    constexpr Bitboard& operator^=(const Bitboard& other) noexcept;

    /** Performs binary shift left (towards higher index positions).
        Zeroes are shifted in, and bits that would go to an index out of range are dropped.
     */
    constexpr Bitboard& operator<<=(size_t num) noexcept;

    /** Performs binary shift right (towards lower index positions).
        Zeroes are shifted in, and bits that would go to an index out of range are dropped.
     */
    constexpr Bitboard& operator>>=(size_t num) noexcept;

    /// @}

    /** Returns a bitboard with only a single bit set. */
    [[nodiscard, gnu::const]] static constexpr Bitboard from_square(const Square& square) noexcept;

private:
    Integer value { 0 };
};

/// @ingroup board
/// @{

/** Returns the binary AND of two bitboards.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr Bitboard operator&(const Bitboard& lhs, const Bitboard& rhs) noexcept;

/** Returns the binary OR of two bitboards.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr Bitboard operator|(const Bitboard& lhs, const Bitboard& rhs) noexcept;

/** Returns the binary XOR of two bitboards.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr Bitboard operator^(const Bitboard& lhs, const Bitboard& rhs) noexcept;

/** Returns a copy of the bitboard with a binary shift left applied.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr Bitboard operator<<(const Bitboard& board, size_t num) noexcept;

/** Returns a copy of the bitboard with a binary shift right applied.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr Bitboard operator>>(const Bitboard& board, size_t num) noexcept;

/** Creates an ASCII representation of the given bitboard.
    The returned string is meant to be interpreted visually by a human, probably for debugging purposes.
    The bitboard is drawn as a simple set of cells separated by ``|`` characters. Occupied squares have an
    ``x`` placed in them.

    @relates Bitboard
 */
[[nodiscard]] std::string print_ascii(Bitboard board);

/// @}

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

constexpr Bitboard::Bitboard(const Integer val) noexcept
    : value { val }
{
}

constexpr Bitboard Bitboard::from_square(const Square& square) noexcept
{
    return Bitboard { 1ULL << square.index() };
}

constexpr bool Bitboard::test(const BitboardIndex index) const noexcept
{
    assert(index <= MAX_BITBOARD_IDX);
    return ((value >> index) & 1uz) != 0;
}

constexpr void Bitboard::set(const BitboardIndex index) noexcept
{
    assert(index <= MAX_BITBOARD_IDX);
    value |= 1ULL << index;
}

constexpr void Bitboard::unset(const BitboardIndex index) noexcept
{
    assert(index <= MAX_BITBOARD_IDX);

    const Integer mask { 1ULL << index };

    value &= ~mask;
}

constexpr BitboardIndex Bitboard::first() const noexcept
{
    // same as number of leading zeroes
    return static_cast<BitboardIndex>(std::countr_zero(value));
}

constexpr BitboardIndex Bitboard::last() const noexcept
{
    if (none()) {
        [[unlikely]];
        return NUM_SQUARES;
    }

    const auto trailingZeroes = static_cast<BitboardIndex>(std::countl_zero(value));

    return NUM_SQUARES - trailingZeroes - static_cast<BitboardIndex>(1);
}

constexpr Bitboard& Bitboard::operator&=(const Bitboard& other) noexcept
{
    value &= other.value;
    return *this;
}

constexpr Bitboard& Bitboard::operator|=(const Bitboard& other) noexcept
{
    value |= other.value;
    return *this;
}

constexpr Bitboard& Bitboard::operator^=(const Bitboard& other) noexcept
{
    value ^= other.value;
    return *this;
}

constexpr Bitboard& Bitboard::operator<<=(const size_t num) noexcept
{
    value <<= num;
    return *this;
}

constexpr Bitboard& Bitboard::operator>>=(const size_t num) noexcept
{
    value >>= num;
    return *this;
}

constexpr Bitboard Bitboard::inverse() const noexcept
{
    return Bitboard { ~value };
}

constexpr Bitboard operator&(const Bitboard& lhs, const Bitboard& rhs) noexcept
{
    auto ret = lhs;
    ret &= rhs;
    return ret;
}

constexpr Bitboard operator|(const Bitboard& lhs, const Bitboard& rhs) noexcept
{
    auto ret = lhs;
    ret |= rhs;
    return ret;
}

constexpr Bitboard operator^(const Bitboard& lhs, const Bitboard& rhs) noexcept
{
    auto ret = lhs;
    ret ^= rhs;
    return ret;
}

constexpr Bitboard operator<<(const Bitboard& board, const size_t num) noexcept
{
    auto ret = board;
    ret <<= num;
    return ret;
}

constexpr Bitboard operator>>(const Bitboard& board, const size_t num) noexcept
{
    auto ret = board;
    ret >>= num;
    return ret;
}

namespace detail {

    // an STL iterator that iterates the set bits in a bitboard, quickly scanning through them using bitscan
    struct BitboardIterator final {
        using value_type   = BitboardIndex;
        using element_type = value_type;
        using pointer      = value_type;
        using reference    = value_type;

        using difference_type = std::ptrdiff_t;

        using iterator_category = std::forward_iterator_tag;
        using iterator_concept  = std::forward_iterator_tag;

        constexpr BitboardIterator() = default;

        explicit constexpr BitboardIterator(const Bitboard& bitboard)
            : value { bitboard.to_int() }
        {
        }

        constexpr bool operator==(const BitboardIterator& other) const noexcept { return value == other.value; }

        [[nodiscard]] constexpr value_type operator*() const noexcept
        {
            return std::countr_zero(value);
        }

        constexpr BitboardIterator& operator++() noexcept
        {
            value &= value - 1;
            return *this;
        }

        [[nodiscard]] constexpr BitboardIterator operator++(int) noexcept
        {
            const auto ret { *this };
            ++*this;
            return ret;
        }

    private:
        std::uint64_t value {};
    };

} // namespace detail

constexpr auto Bitboard::indices() const noexcept
{
    return std::ranges::subrange {
        detail::BitboardIterator { *this },
        detail::BitboardIterator {},
        count()
    };
}

constexpr auto Bitboard::squares() const noexcept
{
    return indices()
         | std::views::transform(
             [](const BitboardIndex index) { return Square::from_index(index); });
}

constexpr auto Bitboard::subboards() const noexcept
{
    return indices()
         | std::views::transform(
             [](const BitboardIndex index) {
                 Bitboard board;
                 board.set(index);
                 return board;
             });
}

} // namespace chess::board
