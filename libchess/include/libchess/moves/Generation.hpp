/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for generating possible moves for each piece type
    from a given starting square. These functions are not concerned with captures
    or blocking pieces, they deal purely with the piece's movement mechanics - i.e.,
    "attacks on an otherwise empty board".

    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>

namespace chess::moves {

using board::Bitboard;
using board::Square;
using pieces::Color;

/// @ingroup moves
/// @{

/** Calculates all possible knight moves from the given starting square.

    The following diagram shows how the bitboard index changes with each possible direction:
    @verbatim
        noNoWe    noNoEa
            +15  +17
             |     |
noWeWe  +6 __|     |__+10  noEaEa
              \   /
               >0<
           __ /   \ __
soWeWe -10   |     |   -6  soEaEa
             |     |
            -17  -15
        soSoWe    soSoEa
    @endverbatim
 */
[[nodiscard, gnu::const]] constexpr Bitboard knight(const Square& starting) noexcept;

/** Calculates all possible bishop moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard bishop(const Square& starting) noexcept;

/** Calculates all possible rook moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard rook(const Square& starting) noexcept;

/** Calculates all possible queen moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard queen(const Square& starting) noexcept;

/** Calculates all possible king moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard king(const Square& starting) noexcept;

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

constexpr Bitboard knight(const Square& starting) noexcept
{
    namespace file_masks = board::masks::files;

    static constexpr auto notAFile  = file_masks::a().inverse();
    static constexpr auto notHFile  = file_masks::h().inverse();
    static constexpr auto notABFile = (file_masks::a() | file_masks::b()).inverse();
    static constexpr auto notGHFile = (file_masks::g() | file_masks::h()).inverse();

    const Bitboard startPos { starting };

    Bitboard moves;

    moves |= (startPos & notHFile) << 17uz;  // noNoEa
    moves |= (startPos & notGHFile) << 10uz; // noEaEa
    moves |= (startPos & notGHFile) >> 6uz;  // soEaEa
    moves |= (startPos & notHFile) >> 15uz;  // soSoEa
    moves |= (startPos & notAFile) << 15uz;  // noNoWe
    moves |= (startPos & notABFile) << 6uz;  // noWeWe
    moves |= (startPos & notABFile) >> 10uz; // soWeWe
    moves |= (startPos & notAFile) >> 17uz;  // soSoWe

    return moves;
}

constexpr Bitboard rook(const Square& starting) noexcept
{
    const auto rankMask = board::masks::ranks::get(starting.rank);
    const auto fileMask = board::masks::files::get(starting.file);

    const auto notStartingSquare = Bitboard { starting }.inverse();

    return (rankMask | fileMask) & notStartingSquare;
}

constexpr Bitboard bishop(const Square& starting) noexcept
{
    const auto diagMask = [starting] {
        static constexpr Bitboard mainDiagonal { 0x8040201008040201 };

        const auto diag = static_cast<int>(starting.file) - static_cast<int>(starting.rank);

        if (diag >= 0)
            return mainDiagonal >> diag * 8;

        return mainDiagonal << -diag * 8;
    }();

    const auto antiDiagMask = [starting] {
        static constexpr Bitboard mainDiagonal { 0x0102040810204080 };

        const auto diag = 7 - static_cast<int>(starting.file) - static_cast<int>(starting.rank);

        if (diag >= 0)
            return mainDiagonal >> diag * 8;

        return mainDiagonal << -diag * 8;
    }();

    const auto notStartingSquare = Bitboard { starting }.inverse();

    return (diagMask | antiDiagMask) & notStartingSquare;
}

constexpr Bitboard queen(const Square& starting) noexcept
{
    return rook(starting) | bishop(starting);
}

namespace detail {

    [[nodiscard, gnu::const]] constexpr Bitboard shift_north(const Bitboard& board) noexcept
    {
        return board << 8uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_south(const Bitboard& board) noexcept
    {
        return board >> 8uz;
    }

    namespace file_masks = board::masks::files;
    namespace rank_masks = board::masks::ranks;

    static constexpr auto notAFile = file_masks::a().inverse();
    static constexpr auto notHFile = file_masks::h().inverse();

    [[nodiscard, gnu::const]] constexpr Bitboard shift_east(const Bitboard& board) noexcept
    {
        return (board & notHFile) << 1uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_west(const Bitboard& board) noexcept
    {
        return (board & notAFile) >> 1uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_northeast(const Bitboard& board) noexcept
    {
        return (board & notHFile) << 9uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_northwest(const Bitboard& board) noexcept
    {
        return (board & notAFile) << 7uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_southeast(const Bitboard& board) noexcept
    {
        return (board & notHFile) >> 7uz;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard shift_southwest(const Bitboard& board) noexcept
    {
        return (board & notAFile) >> 9uz;
    }

    // TODO: top-level interface for pawn moves? Or query each subtype individually?

    [[nodiscard, gnu::const]] constexpr Bitboard pawn_pushes(
        const Bitboard& starting, const Color color) noexcept
    {
        if (color == Color::White)
            return shift_north(starting);

        return shift_south(starting);
    }

    [[nodiscard, gnu::const]] constexpr Bitboard pawn_double_pushes(
        const Bitboard& starting, const Color color) noexcept
    {
        if (color == Color::White)
            return (starting << 16uz) & rank_masks::four();

        return (starting >> 16uz) & rank_masks::five();
    }

    // calculates all squares attacked by a pawn
    [[nodiscard, gnu::const]] constexpr Bitboard pawn_attacks(
        const Bitboard& starting, const Color color) noexcept
    {
        if (color == Color::White)
            return shift_northeast(starting) | shift_northwest(starting);

        return shift_southeast(starting) | shift_southwest(starting);
    }

} // namespace detail

constexpr Bitboard king(const Square& starting) noexcept
{
    Bitboard startPos { starting };

    auto moves = detail::shift_east(startPos) | detail::shift_west(startPos);

    startPos |= moves;

    moves |= detail::shift_north(startPos) | detail::shift_south(startPos);

    return moves;
}

} // namespace chess::moves
