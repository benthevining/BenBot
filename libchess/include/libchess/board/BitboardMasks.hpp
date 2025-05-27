/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup bitboard_masks Bitboard masks
    Compile-time bitboard constants and masks.
    @ingroup board
 */

/** @file
    This file provides some useful compile-time bitboard constants and masks.
    @ingroup bitboard_masks
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace provides some compile-time bitboard constants and masks.
    @ingroup bitboard_masks
 */
namespace chess::board::masks {

using std::size_t;

/// @ingroup bitboard_masks
/// @{

/** Returns a bitboard with all bits set to 1.
    @see none()
 */
[[nodiscard, gnu::const]] consteval Bitboard all() noexcept
{
    return Bitboard { 0XFFFFFFFFFFFFFFFF };
}

/** Returns a bitboard with all bits set to 0.
    @see all()
 */
[[nodiscard, gnu::const]] consteval Bitboard none() noexcept
{
    return {};
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

/** Returns a bitboard with all the center squares (D4, D5, E4, E5) set to 1. */
[[nodiscard, gnu::const]] consteval Bitboard center() noexcept
{
    return Bitboard { 0X1818000000 };
}

/** Returns a bitboard with all the perimeter squares set to 1. */
[[nodiscard, gnu::const]] consteval Bitboard perimeter() noexcept
{
    return Bitboard { 0XFF818181818181FF };
}

/** Returns a bitboard with all squares on the same diagonal as the given square set to 1. */
[[nodiscard, gnu::const]] constexpr Bitboard diagonal(const Square& square) noexcept
{
    const auto diag = static_cast<int>(square.file) - static_cast<int>(square.rank);

    if (diag >= 0)
        return a1_h8_diagonal() >> diag * 8;

    return a1_h8_diagonal() << -diag * 8;
}

/** Returns a bitboard with all squares on the same antidiagonal as the given square set to 1. */
[[nodiscard, gnu::const]] constexpr Bitboard antidiagonal(const Square& square) noexcept
{
    const auto diag = 7 - static_cast<int>(square.file) - static_cast<int>(square.rank);

    if (diag >= 0)
        return a8_h1_diagonal() >> diag * 8;

    return a8_h1_diagonal() << -diag * 8;
}

/// @}

/** This namespace provides some compile-time bitboard masks for files.
    @ingroup bitboard_masks
 */
namespace files {

    /// @ingroup bitboard_masks
    /// @{

    /** Returns a bitboard with all squares on the A file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard a() noexcept
    {
        return Bitboard { 0x0101010101010101 };
    }

    /** Returns a bitboard with all squares on the B file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard b() noexcept
    {
        return Bitboard { 0X202020202020202 };
    }

    /** Returns a bitboard with all squares on the C file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard c() noexcept
    {
        return Bitboard { 0X404040404040404 };
    }

    /** Returns a bitboard with all squares on the D file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard d() noexcept
    {
        return Bitboard { 0X808080808080808 };
    }

    /** Returns a bitboard with all squares on the E file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard e() noexcept
    {
        return Bitboard { 0X1010101010101010 };
    }

    /** Returns a bitboard with all squares on the F file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard f() noexcept
    {
        return Bitboard { 0X2020202020202020 };
    }

    /** Returns a bitboard with all squares on the G file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard g() noexcept
    {
        return Bitboard { 0X4040404040404040 };
    }

    /** Returns a bitboard with all squares on the H file set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard h() noexcept
    {
        return Bitboard { 0x8080808080808080 };
    }

    /** Returns a bitboard with all squares on the requested file set to 1. */
    [[nodiscard, gnu::const]] constexpr Bitboard get(const File file) noexcept
    {
        return a() << static_cast<size_t>(file);
    }

    /// @}

} // namespace files

/** This namespace provides some compile-time bitboard masks for ranks.
    @ingroup bitboard_masks
 */
namespace ranks {

    /// @ingroup bitboard_masks
    /// @{

    /** Returns a bitboard with all squares on the first rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard one() noexcept
    {
        return Bitboard { 0x00000000000000FF };
    }

    /** Returns a bitboard with all squares on the second rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard two() noexcept
    {
        return Bitboard { 0XFF00 };
    }

    /** Returns a bitboard with all squares on the third rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard three() noexcept
    {
        return Bitboard { 0XFF0000 };
    }

    /** Returns a bitboard with all squares on the fourth rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard four() noexcept
    {
        return Bitboard { 0XFF000000 };
    }

    /** Returns a bitboard with all squares on the fifth rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard five() noexcept
    {
        return Bitboard { 0XFF00000000 };
    }

    /** Returns a bitboard with all squares on the sixth rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard six() noexcept
    {
        return Bitboard { 0XFF0000000000 };
    }

    /** Returns a bitboard with all squares on the seventh rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard seven() noexcept
    {
        return Bitboard { 0XFF000000000000 };
    }

    /** Returns a bitboard with all squares on the eighth rank set to 1. */
    [[nodiscard, gnu::const]] consteval Bitboard eight() noexcept
    {
        return Bitboard { 0xFF00000000000000 };
    }

    /** Returns a bitboard with all squares on the requested rank set to 1. */
    [[nodiscard, gnu::const]] constexpr Bitboard get(const Rank rank) noexcept
    {
        // TODO - could be branchless:
        // Bitboard{0xff} << (index & 56)

        switch (rank) {
            case Rank::Two  : return two();
            case Rank::Three: return three();
            case Rank::Four : return four();
            case Rank::Five : return five();
            case Rank::Six  : return six();
            case Rank::Seven: return seven();
            case Rank::Eight: return eight();
            default         : return one();
        }
    }

    /// @}

} // namespace ranks

/** This namespace provides some compile-time bitboard constants for the starting positions
    of each piece type.

    @ingroup bitboard_masks
 */
namespace starting {

    /** This namespace provides compile-time bitboard constants for the starting locations of
        the White pieces.

        @ingroup bitboard_masks
        @see black
     */
    namespace white {

        /// @ingroup bitboard_masks
        /// @{

        /** Returns a bitboard mask for the starting position of White's pawns. */
        [[nodiscard, gnu::const]] consteval Bitboard pawns() noexcept
        {
            return ranks::two();
        }

        /** Returns a bitboard mask for the starting position of White's rooks. */
        [[nodiscard, gnu::const]] consteval Bitboard rooks() noexcept
        {
            return Bitboard { 0X81 };
        }

        /** Returns a bitboard mask for the starting position of White's knights. */
        [[nodiscard, gnu::const]] consteval Bitboard knights() noexcept
        {
            return Bitboard { 0X42 };
        }

        /** Returns a bitboard mask for the starting position of White's bishops. */
        [[nodiscard, gnu::const]] consteval Bitboard bishops() noexcept
        {
            return Bitboard { 0X24 };
        }

        /** Returns a bitboard mask for the starting position of White's queen. */
        [[nodiscard, gnu::const]] consteval Bitboard queen() noexcept
        {
            return Bitboard { 0X8 };
        }

        /** Returns a bitboard mask for the starting position of White's king. */
        [[nodiscard, gnu::const]] consteval Bitboard king() noexcept
        {
            return Bitboard { 0X10 };
        }

        /// @}

    } // namespace white

    /** This namespace provides compile-time bitboard constants for the starting locations of
        the Black pieces.

        @ingroup bitboard_masks
        @see white
     */
    namespace black {

        /// @ingroup bitboard_masks
        /// @{

        /** Returns a bitboard mask for the starting position of Black's pawns. */
        [[nodiscard, gnu::const]] consteval Bitboard pawns() noexcept
        {
            return ranks::seven();
        }

        /** Returns a bitboard mask for the starting position of Black's rooks. */
        [[nodiscard, gnu::const]] consteval Bitboard rooks() noexcept
        {
            return Bitboard { 0X8100000000000000 };
        }

        /** Returns a bitboard mask for the starting position of Black's knights. */
        [[nodiscard, gnu::const]] consteval Bitboard knights() noexcept
        {
            return Bitboard { 0X4200000000000000 };
        }

        /** Returns a bitboard mask for the starting position of Black's bishops. */
        [[nodiscard, gnu::const]] consteval Bitboard bishops() noexcept
        {
            return Bitboard { 0X2400000000000000 };
        }

        /** Returns a bitboard mask for the starting position of Black's queen. */
        [[nodiscard, gnu::const]] consteval Bitboard queen() noexcept
        {
            return Bitboard { 0X800000000000000 };
        }

        /** Returns a bitboard mask for the starting position of Black's king. */
        [[nodiscard, gnu::const]] consteval Bitboard king() noexcept
        {
            return Bitboard { 0X1000000000000000 };
        }

        /// @}

    } // namespace black

    /// @ingroup bitboard_masks
    /// @{

    /** Returns a bitboard mask for the starting position of the pawns for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard pawns(const Color color) noexcept
    {
        if (color == Color::White)
            return white::pawns();

        return black::pawns();
    }

    /** Returns a bitboard mask for the starting position of the rooks for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard rooks(const Color color) noexcept
    {
        if (color == Color::White)
            return white::rooks();

        return black::rooks();
    }

    /** Returns a bitboard mask for the starting position of the knights for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard knights(const Color color) noexcept
    {
        if (color == Color::White)
            return white::knights();

        return black::knights();
    }

    /** Returns a bitboard mask for the starting position of the bishops for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard bishops(const Color color) noexcept
    {
        if (color == Color::White)
            return white::bishops();

        return black::bishops();
    }

    /** Returns a bitboard mask for the starting position of the queen for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard queen(const Color color) noexcept
    {
        if (color == Color::White)
            return white::queen();

        return black::queen();
    }

    /** Returns a bitboard mask for the starting position of the king for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard king(const Color color) noexcept
    {
        if (color == Color::White)
            return white::king();

        return black::king();
    }

    /// @}

} // namespace starting

} // namespace chess::board::masks
