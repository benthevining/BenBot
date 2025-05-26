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

/// @}

// pawns
// bishops
// rooks
// queens
// king

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
    static constexpr auto notGHFile = (file_masks::h() | file_masks::g()).inverse();
    static constexpr auto notABFile = (file_masks::a() | file_masks::b()).inverse();

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

} // namespace chess::moves
