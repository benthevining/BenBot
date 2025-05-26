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
    or blocking pieces, they deal purely with the piece's movement mechanics.

    @ingroup moves

    @todo pawns
    @todo bishops
    @todo rooks
    @todo queens
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Square.hpp>

namespace chess::moves {

using board::Bitboard;
using board::Square;

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

constexpr Bitboard king(const Square& starting) noexcept
{
    namespace file_masks = board::masks::files;

    auto shift_east = [](const Bitboard& board) {
        static constexpr auto notHFile = file_masks::h().inverse();

        return (board & notHFile) << 1uz;
    };

    auto shift_west = [](const Bitboard& board) {
        static constexpr auto notAFile = file_masks::a().inverse();

        return (board & notAFile) >> 1uz;
    };

    auto shift_north = [](const Bitboard& board) {
        return board << 8uz;
    };

    auto shift_south = [](const Bitboard& board) {
        return board >> 8uz;
    };

    Bitboard startPos { starting };

    auto moves = shift_east(startPos) | shift_west(startPos);

    startPos |= moves;

    moves |= shift_north(startPos) | shift_south(startPos);

    return moves;
}

} // namespace chess::moves
