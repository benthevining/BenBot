/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
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
#include <iterator>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Square.hpp>
#include <ranges>
#include <string>
#include <utility>

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
    [[nodiscard]] constexpr auto operator==(const Bitboard&) const noexcept -> bool = default;

    /// @name Observers
    /// @{

    /** Returns true if any of the bits are set. */
    [[nodiscard]] constexpr auto any() const noexcept -> bool { return std::cmp_not_equal(value, 0); }

    /** Returns true if none of the bits are set. */
    [[nodiscard]] constexpr auto none() const noexcept -> bool { return std::cmp_equal(value, 0); }

    /** Returns the number of bits that are set. */
    [[nodiscard]] constexpr auto count() const noexcept -> size_t { return static_cast<size_t>(std::popcount(value)); }

    /** Returns true if there is a piece on the given square. */
    [[nodiscard]] constexpr auto test(const Square square) const noexcept -> bool { return test(square.index()); }

    /** Returns true if there is a piece on the given square.
        This method asserts if the given index is greater than 63.
     */
    [[nodiscard]] constexpr auto test(BitboardIndex index) const noexcept -> bool;

    /** Returns the index of the first set bit.
        This operation may also be known as "bitscan forward".
        Returns 64 if all bits are 0.
     */
    [[nodiscard]] constexpr auto first() const noexcept -> BitboardIndex;

    /** Returns the index of the last set bit.
        This operation may also be known as "bitscan reverse".
        Returns 64 if all bits are 0.
     */
    [[nodiscard]] constexpr auto last() const noexcept -> BitboardIndex;

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
    constexpr void clear() noexcept { value = UINT64_C(0); }

    /// @}

    /** Converts this bitboard to its integer representation. */
    [[nodiscard]] constexpr auto to_int() const noexcept -> Integer { return value; }

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
    [[nodiscard]] constexpr auto inverse() const noexcept -> Bitboard;

    /** Performs binary AND with another bitboard. */
    constexpr auto operator&=(const Bitboard& other) noexcept -> Bitboard&;

    /** Performs binary OR with another bitboard. */
    constexpr auto operator|=(const Bitboard& other) noexcept -> Bitboard&;

    /** Performs binary XOR with another bitboard. */
    constexpr auto operator^=(const Bitboard& other) noexcept -> Bitboard&;

    /** Performs binary shift left (towards higher index positions).
        Zeroes are shifted in, and bits that would go to an index out of range are dropped.
     */
    constexpr auto operator<<=(size_t num) noexcept -> Bitboard&;

    /** Performs binary shift right (towards lower index positions).
        Zeroes are shifted in, and bits that would go to an index out of range are dropped.
     */
    constexpr auto operator>>=(size_t num) noexcept -> Bitboard&;

    /// @}

    /** Returns a bitboard with only a single bit set. */
    [[nodiscard, gnu::const]] static constexpr auto from_square(Square square) noexcept -> Bitboard;

private:
    Integer value { UINT64_C(0) };
};

/// @ingroup board
/// @{

/** Returns the binary AND of two bitboards.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr auto operator&(const Bitboard& lhs, const Bitboard& rhs) noexcept -> Bitboard;

/** Returns the binary OR of two bitboards.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr auto operator|(const Bitboard& lhs, const Bitboard& rhs) noexcept -> Bitboard;

/** Returns the binary XOR of two bitboards.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr auto operator^(const Bitboard& lhs, const Bitboard& rhs) noexcept -> Bitboard;

/** Returns a copy of the bitboard with a binary shift left applied.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr auto operator<<(const Bitboard& board, size_t num) noexcept -> Bitboard;

/** Returns a copy of the bitboard with a binary shift right applied.
    @relates Bitboard
 */
[[nodiscard, gnu::const]] constexpr auto operator>>(const Bitboard& board, size_t num) noexcept -> Bitboard;

/** Creates an ASCII representation of the given bitboard.
    The returned string is meant to be interpreted visually by a human, probably for debugging purposes.
    The bitboard is drawn as a simple set of cells separated by ``|`` characters. Occupied squares have an
    ``x`` placed in them.

    @relates Bitboard
 */
[[nodiscard]] auto print_ascii(Bitboard board) -> std::string;

/// @}

/** This namespace contains user-defined literal operators for applicable types in the ``chess::board`` namespace.
    @ingroup board
 */
namespace literals {

    /** Creates a bitboard from an integer literal value.
        @ingroup board
        @relates chess::board::Bitboard
     */
    [[nodiscard, gnu::const]] consteval auto operator""_bb(
        const unsigned long long value) noexcept // NOLINT(runtime/int)
        -> Bitboard
    {
        return Bitboard { static_cast<Bitboard::Integer>(value) };
    }

} // namespace literals

} // namespace chess::board

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

namespace chess::board {

constexpr Bitboard::Bitboard(const Integer val) noexcept
    : value { val }
{
}

constexpr auto Bitboard::from_square(const Square square) noexcept -> Bitboard
{
    return Bitboard { UINT64_C(1) << square.index() };
}

constexpr auto Bitboard::test(const BitboardIndex index) const noexcept -> bool
{
    assert(std::cmp_less_equal(index, MAX_BITBOARD_IDX));

    return std::cmp_not_equal(
        (value >> index) & UINT64_C(1),
        0);
}

constexpr void Bitboard::set(const BitboardIndex index) noexcept
{
    assert(std::cmp_less_equal(index, MAX_BITBOARD_IDX));

    value |= UINT64_C(1) << index;
}

constexpr void Bitboard::unset(const BitboardIndex index) noexcept
{
    assert(std::cmp_less_equal(index, MAX_BITBOARD_IDX));

    const Integer mask { UINT64_C(1) << index };

    value &= ~mask;
}

constexpr auto Bitboard::first() const noexcept -> BitboardIndex
{
    // same as number of leading zeroes
    return static_cast<BitboardIndex>(std::countr_zero(value));
}

constexpr auto Bitboard::last() const noexcept -> BitboardIndex
{
    if (none()) {
        [[unlikely]]; // this is probably an error condition, but we want this function to be noexcept
        return NUM_SQUARES;
    }

    const auto trailingZeroes = static_cast<BitboardIndex>(std::countl_zero(value));

    return NUM_SQUARES - trailingZeroes - static_cast<BitboardIndex>(1);
}

constexpr auto Bitboard::operator&=(const Bitboard& other) noexcept -> Bitboard&
{
    value &= other.value;
    return *this;
}

constexpr auto Bitboard::operator|=(const Bitboard& other) noexcept -> Bitboard&
{
    value |= other.value;
    return *this;
}

constexpr auto Bitboard::operator^=(const Bitboard& other) noexcept -> Bitboard&
{
    value ^= other.value;
    return *this;
}

constexpr auto Bitboard::operator<<=(const size_t num) noexcept -> Bitboard&
{
    value <<= num;
    return *this;
}

constexpr auto Bitboard::operator>>=(const size_t num) noexcept -> Bitboard&
{
    value >>= num;
    return *this;
}

constexpr auto Bitboard::inverse() const noexcept -> Bitboard
{
    return Bitboard { ~value };
}

constexpr auto operator&(const Bitboard& lhs, const Bitboard& rhs) noexcept -> Bitboard
{
    auto ret = lhs;
    ret &= rhs;
    return ret;
}

constexpr auto operator|(const Bitboard& lhs, const Bitboard& rhs) noexcept -> Bitboard
{
    auto ret = lhs;
    ret |= rhs;
    return ret;
}

constexpr auto operator^(const Bitboard& lhs, const Bitboard& rhs) noexcept -> Bitboard
{
    auto ret = lhs;
    ret ^= rhs;
    return ret;
}

constexpr auto operator<<(const Bitboard& board, const size_t num) noexcept -> Bitboard
{
    auto ret = board;
    ret <<= num;
    return ret;
}

constexpr auto operator>>(const Bitboard& board, const size_t num) noexcept -> Bitboard
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

        consteval BitboardIterator() = default;

        explicit constexpr BitboardIterator(const Bitboard& bitboard)
            : value { bitboard.to_int() }
        {
        }

        constexpr auto operator==(const BitboardIterator& other) const noexcept -> bool = default;

        constexpr auto operator==([[maybe_unused]] std::default_sentinel_t sentinel) const noexcept -> bool
        {
            return std::cmp_equal(value, 0);
        }

        [[nodiscard]] constexpr auto operator*() const noexcept -> value_type
        {
            return static_cast<value_type>(std::countr_zero(value));
        }

        constexpr auto operator++() noexcept -> BitboardIterator&
        {
            assert(std::cmp_greater(value, 0));

            value &= value - UINT64_C(1);

            return *this;
        }

        [[nodiscard]] constexpr auto operator++(int) noexcept -> BitboardIterator
        {
            const auto ret { *this };
            ++*this;
            return ret;
        }

    private:
        Bitboard::Integer value { UINT64_C(0) };
    };

} // namespace detail

constexpr auto Bitboard::indices() const noexcept
{
    return std::ranges::subrange {
        detail::BitboardIterator { *this },
        detail::BitboardIterator { },
        count()
    };
}

constexpr auto Bitboard::squares() const noexcept
{
    return indices()
         | std::views::transform(Square::from_index);
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
