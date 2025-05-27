/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for generating pseudo-legal moves.

    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace provides functions for generating possible moves for each piece type
    from a given starting square. These functions are not concerned with captures
    or blocking pieces, they deal purely with the piece's movement mechanics - i.e.,
    "attacks on an otherwise empty board". The returned move sets do not include the
    starting square.

    @ingroup moves
 */
namespace chess::moves::pseudo_legal {

using board::Bitboard;
using board::Square;
using pieces::Color;

/// @ingroup moves
/// @{

/** Calculates all possible pawn pushes for the given starting position and color. */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_pushes(Bitboard starting, Color color) noexcept;

/** Calculates all possible pawn double pushes for the given starting position and color. */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_double_pushes(Bitboard starting, Color color) noexcept;

/** Calculates all squares that are attacked by pawns from the given starting position and color.
    This can be used to calculate possible pawn captures.
 */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_attacks(Bitboard starting, Color color) noexcept;

/** Calculates all possible knight moves from the given starting position.
    This function can calculate moves for any number of knights.
 */
[[nodiscard, gnu::const]] constexpr Bitboard knight(Bitboard starting) noexcept;

/** Calculates all possible bishop moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard bishop(const Square& starting) noexcept;

/** Calculates all possible rook moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard rook(const Square& starting) noexcept;

/** Calculates all possible queen moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard queen(const Square& starting) noexcept;

/** Calculates all possible king moves from the given starting position.
    Typically the starting bitboard will have only a single bit set, but this method can
    technically calculate moves for both kings at once, if both of their positions are
    set in the bitboard sent to this function.
 */
[[nodiscard, gnu::const]] constexpr Bitboard king(Bitboard starting) noexcept;

/// @}

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

constexpr Bitboard pawn_double_pushes(const Bitboard starting, const Color color) noexcept
{
    namespace rank_masks = board::masks::ranks;

    if (color == Color::White)
        return (starting << 16uz) & rank_masks::four();

    return (starting >> 16uz) & rank_masks::five();
}

constexpr Bitboard knight(const Bitboard starting) noexcept
{
    namespace file_masks = board::masks::files;

    static constexpr auto notAFile  = file_masks::a().inverse();
    static constexpr auto notHFile  = file_masks::h().inverse();
    static constexpr auto notABFile = (file_masks::a() | file_masks::b()).inverse();
    static constexpr auto notGHFile = (file_masks::g() | file_masks::h()).inverse();

    Bitboard moves;

    moves |= (starting & notHFile) << 17uz;  // noNoEa
    moves |= (starting & notGHFile) << 10uz; // noEaEa
    moves |= (starting & notGHFile) >> 6uz;  // soEaEa
    moves |= (starting & notHFile) >> 15uz;  // soSoEa
    moves |= (starting & notAFile) << 15uz;  // noNoWe
    moves |= (starting & notABFile) << 6uz;  // noWeWe
    moves |= (starting & notABFile) >> 10uz; // soWeWe
    moves |= (starting & notAFile) >> 17uz;  // soSoWe

    return moves;
}

constexpr Bitboard rook(const Square& starting) noexcept
{
    const auto rankMask = board::masks::ranks::get(starting.rank);
    const auto fileMask = board::masks::files::get(starting.file);

    const auto notStartingSquare = Bitboard { starting }.inverse();

    return (rankMask | fileMask) & notStartingSquare;
}

namespace detail {

    [[nodiscard, gnu::const]] constexpr Bitboard shift_north(const Bitboard board) noexcept
    {
        return board << 8uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_south(const Bitboard board) noexcept
    {
        return board >> 8uz;
    }

    namespace file_masks = board::masks::files;
    namespace rank_masks = board::masks::ranks;

    static constexpr auto notAFile = file_masks::a().inverse();
    static constexpr auto notHFile = file_masks::h().inverse();

    [[nodiscard, gnu::const]] constexpr Bitboard shift_east(const Bitboard board) noexcept
    {
        return (board & notHFile) << 1uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_west(const Bitboard board) noexcept
    {
        return (board & notAFile) >> 1uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_northeast(const Bitboard board) noexcept
    {
        return (board & notHFile) << 9uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_northwest(const Bitboard board) noexcept
    {
        return (board & notAFile) << 7uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_southeast(const Bitboard board) noexcept
    {
        return (board & notHFile) >> 7uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_southwest(const Bitboard board) noexcept
    {
        return (board & notAFile) >> 9uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard diagonal_mask(const Square& starting) noexcept
    {
        static constexpr Bitboard mainDiagonal { 0x8040201008040201 };

        const auto diag = static_cast<int>(starting.file) - static_cast<int>(starting.rank);

        if (diag >= 0)
            return mainDiagonal >> diag * 8;

        return mainDiagonal << -diag * 8;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard antidiagonal_mask(const Square& starting) noexcept
    {
        static constexpr Bitboard mainDiagonal { 0x0102040810204080 };

        const auto diag = 7 - static_cast<int>(starting.file) - static_cast<int>(starting.rank);

        if (diag >= 0)
            return mainDiagonal >> diag * 8;

        return mainDiagonal << -diag * 8;
    }

} // namespace detail

constexpr Bitboard pawn_pushes(const Bitboard starting, const Color color) noexcept
{
    if (color == Color::White)
        return detail::shift_north(starting);

    return detail::shift_south(starting);
}

constexpr Bitboard pawn_attacks(const Bitboard starting, const Color color) noexcept
{
    if (color == Color::White)
        return detail::shift_northeast(starting) | detail::shift_northwest(starting);

    return detail::shift_southeast(starting) | detail::shift_southwest(starting);
}

constexpr Bitboard bishop(const Square& starting) noexcept
{
    const auto diagMask     = detail::diagonal_mask(starting);
    const auto antiDiagMask = detail::antidiagonal_mask(starting);

    const auto notStartingSquare = Bitboard { starting }.inverse();

    return (diagMask | antiDiagMask) & notStartingSquare;
}

constexpr Bitboard queen(const Square& starting) noexcept
{
    const auto rankMask     = board::masks::ranks::get(starting.rank);
    const auto fileMask     = board::masks::files::get(starting.file);
    const auto diagMask     = detail::diagonal_mask(starting);
    const auto antiDiagMask = detail::antidiagonal_mask(starting);

    const auto notStartingSquare = Bitboard { starting }.inverse();

    return (rankMask | fileMask | diagMask | antiDiagMask) & notStartingSquare;
}

constexpr Bitboard king(Bitboard starting) noexcept
{
    auto moves = detail::shift_east(starting) | detail::shift_west(starting);

    starting |= moves;

    moves |= detail::shift_north(starting) | detail::shift_south(starting);

    return moves;
}

} // namespace chess::moves::pseudo_legal
