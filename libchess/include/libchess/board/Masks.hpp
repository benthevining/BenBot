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
#include <utility>

/** This namespace provides some compile-time bitboard constants and masks.
    @ingroup bitboard_masks
 */
namespace chess::board::masks {

using pieces::Color;
using std::size_t;

/// @ingroup bitboard_masks
/// @{

/** A bitboard with all bits set to 1. */
static constexpr Bitboard ALL { 0XFFFFFFFFFFFFFFFF };

/** A bitboard with all bits set to 0. */
static constexpr Bitboard NONE {};

/** A bitboard with all the dark squares set to 1. */
static constexpr Bitboard DARK_SQUARES { 0xAA55AA55AA55AA55 };

/** A bitboard with all the light squares set to 1. */
static constexpr Bitboard LIGHT_SQUARES { 0x55AA55AA55AA55AA };

/** A bitboard with all squares on the A1-H8 long diagonal set to 1. */
static constexpr Bitboard MAIN_DIAGONAL { 0x8040201008040201 };

/** A bitboard with all squares on the A1-H8 long anti-diagonal set to 1. */
static constexpr Bitboard MAIN_ANTIDIAGONAL { 0x0102040810204080 };

/** A bitboard with the 4 center squares (D4, D5, E4, E5) set to 1. */
static constexpr Bitboard CENTER { 0X1818000000 };

/** A bitboard with all the perimeter squares set to 1. */
static constexpr Bitboard PERIMETER { 0XFF818181818181FF };

/** Returns a bitboard with all squares on the same diagonal as the given square set to 1. */
[[nodiscard, gnu::const]] constexpr Bitboard diagonal(const Square& square) noexcept
{
    const auto diag = static_cast<int>(square.file) - static_cast<int>(square.rank);

    if (diag >= 0)
        return MAIN_DIAGONAL >> static_cast<size_t>(diag) * 8uz;

    return MAIN_DIAGONAL << static_cast<size_t>(-diag) * 8uz;
}

/** Returns a bitboard with all squares on the same antidiagonal as the given square set to 1. */
[[nodiscard, gnu::const]] constexpr Bitboard antidiagonal(const Square& square) noexcept
{
    const auto diag = 7 - static_cast<int>(square.file) - static_cast<int>(square.rank);

    if (diag >= 0)
        return MAIN_ANTIDIAGONAL >> static_cast<size_t>(diag) * 8uz;

    return MAIN_ANTIDIAGONAL << static_cast<size_t>(-diag) * 8uz;
}

/** Returns a bitboard mask with the starting & ending rook positions of a queenside castling
    move. This mask can be XOR'ed with the rooks bitboard to update the rook position after
    queenside castling.

    @see kingside_castle_rook_pos_mask()
 */
[[nodiscard, gnu::const]] constexpr Bitboard queenside_castle_rook_pos_mask(
    const Color side) noexcept
{
    const auto rank = side == Color::White ? Rank::One : Rank::Eight;

    Bitboard mask;

    mask.set(Square { .file = File::A, .rank = rank });
    mask.set(Square { .file = File::D, .rank = rank });

    return mask;
}

/** Returns a bitboard mask with the starting & ending rook positions of a kingside castling
    move. This mask can be XOR'ed with the rooks bitboard to update the rook position after
    kingside castling.

    @see queenside_castle_rook_pos_mask()
 */
[[nodiscard, gnu::const]] constexpr Bitboard kingside_castle_rook_pos_mask(
    const Color side) noexcept
{
    const auto rank = side == Color::White ? Rank::One : Rank::Eight;

    Bitboard mask;

    mask.set(Square { .file = File::H, .rank = rank });
    mask.set(Square { .file = File::F, .rank = rank });

    return mask;
}

/// @}

/** This namespace provides some compile-time bitboard masks for files.
    @ingroup bitboard_masks
 */
namespace files {

    /// @ingroup bitboard_masks
    /// @{

    /** A bitboard with all squares on the A file set to 1. */
    static constexpr Bitboard A { 0x0101010101010101 };

    /** A bitboard with all squares on the B file set to 1. */
    static constexpr Bitboard B { 0X202020202020202 };

    /** A bitboard with all squares on the C file set to 1. */
    static constexpr Bitboard C { 0X404040404040404 };

    /** A bitboard with all squares on the D file set to 1. */
    static constexpr Bitboard D { 0X808080808080808 };

    /** A bitboard with all squares on the E file set to 1. */
    static constexpr Bitboard E { 0X1010101010101010 };

    /** A bitboard with all squares on the F file set to 1. */
    static constexpr Bitboard F { 0X2020202020202020 };

    /** A bitboard with all squares on the G file set to 1. */
    static constexpr Bitboard G { 0X4040404040404040 };

    /** A bitboard with all squares on the H file set to 1. */
    static constexpr Bitboard H { 0x8080808080808080 };

    /** Returns a bitboard with all squares on the requested file set to 1. */
    [[nodiscard, gnu::const]] constexpr Bitboard get(const File file) noexcept
    {
        return A << static_cast<size_t>(file);
    }

    /// @}

} // namespace files

/** This namespace provides some compile-time bitboard masks for ranks.
    @ingroup bitboard_masks
 */
namespace ranks {

    /// @ingroup bitboard_masks
    /// @{

    /** A bitboard with all squares on the first rank set to 1. */
    static constexpr Bitboard ONE { 0x00000000000000FF };

    /** A bitboard with all squares on the second rank set to 1. */
    static constexpr Bitboard TWO { 0XFF00 };

    /** A bitboard with all squares on the third rank set to 1. */
    static constexpr Bitboard THREE { 0XFF0000 };

    /** A bitboard with all squares on the fourth rank set to 1. */
    static constexpr Bitboard FOUR { 0XFF000000 };

    /** A bitboard with all squares on the fifth rank set to 1. */
    static constexpr Bitboard FIVE { 0XFF00000000 };

    /** A bitboard with all squares on the sixth rank set to 1. */
    static constexpr Bitboard SIX { 0XFF0000000000 };

    /** A bitboard with all squares on the seventh rank set to 1. */
    static constexpr Bitboard SEVEN { 0XFF000000000000 };

    /** A bitboard with all squares on the eighth rank set to 1. */
    static constexpr Bitboard EIGHT { 0xFF00000000000000 };

    /** Returns a bitboard with all squares on the requested rank set to 1. */
    [[nodiscard, gnu::const]] constexpr Bitboard get(const Rank rank) noexcept
    {
        return ONE << (8uz * std::to_underlying(rank));
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

        /** A bitboard mask for the starting position of White's pawns. */
        static constexpr Bitboard PAWNS { ranks::TWO };

        /** A bitboard mask for the starting position of White's rooks. */
        static constexpr Bitboard ROOKS { 0X81 };

        /** A bitboard mask for the starting position of White's knights. */
        static constexpr Bitboard KNIGHTS { 0X42 };

        /** A bitboard mask for the starting position of White's bishops. */
        static constexpr Bitboard BISHOPS { 0X24 };

        /** A bitboard mask for the starting position of White's queen. */
        static constexpr Bitboard QUEEN { 0X8 };

        /** A bitboard mask for the starting position of White's king. */
        static constexpr Bitboard KING { 0X10 };

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

        /** A bitboard mask for the starting position of Black's pawns. */
        static constexpr Bitboard PAWNS { ranks::SEVEN };

        /** A bitboard mask for the starting position of Black's rooks. */
        static constexpr Bitboard ROOKS { 0X8100000000000000 };

        /** A bitboard mask for the starting position of Black's knights. */
        static constexpr Bitboard KNIGHTS { 0X4200000000000000 };

        /** A bitboard mask for the starting position of Black's bishops. */
        static constexpr Bitboard BISHOPS { 0X2400000000000000 };

        /** A bitboard mask for the starting position of Black's queen. */
        static constexpr Bitboard QUEEN { 0X800000000000000 };

        /** A bitboard mask for the starting position of Black's king. */
        static constexpr Bitboard KING { 0X1000000000000000 };

        /// @}

    } // namespace black

    /// @ingroup bitboard_masks
    /// @{

    /** Returns a bitboard mask for the starting position of the pawns for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard pawns(const Color color) noexcept
    {
        if (color == Color::White)
            return white::PAWNS;

        return black::PAWNS;
    }

    /** Returns a bitboard mask for the starting position of the rooks for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard rooks(const Color color) noexcept
    {
        if (color == Color::White)
            return white::ROOKS;

        return black::ROOKS;
    }

    /** Returns a bitboard mask for the starting position of the knights for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard knights(const Color color) noexcept
    {
        if (color == Color::White)
            return white::KNIGHTS;

        return black::KNIGHTS;
    }

    /** Returns a bitboard mask for the starting position of the bishops for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard bishops(const Color color) noexcept
    {
        if (color == Color::White)
            return white::BISHOPS;

        return black::BISHOPS;
    }

    /** Returns a bitboard mask for the starting position of the queen for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard queen(const Color color) noexcept
    {
        if (color == Color::White)
            return white::QUEEN;

        return black::QUEEN;
    }

    /** Returns a bitboard mask for the starting position of the king for the given side. */
    [[nodiscard, gnu::const]] constexpr Bitboard king(const Color color) noexcept
    {
        if (color == Color::White)
            return white::KING;

        return black::KING;
    }

    /// @}

} // namespace starting

} // namespace chess::board::masks
