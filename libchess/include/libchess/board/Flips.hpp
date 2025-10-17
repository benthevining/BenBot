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
    This file provides functions for flipping bitboards vertically or horizontally.
    @ingroup board
 */

#pragma once

#include <bit>
#include <libchess/board/Bitboard.hpp>

/** This namespace contains functions for flipping bitboards vertically or horizontally.
    @ingroup board
 */
namespace chess::board::flips {

/** Returns a copy of the bitboard, mirrored vertically.
    @ingroup board
 */
[[nodiscard, gnu::const]] constexpr auto vertical(Bitboard board) noexcept -> Bitboard;

/** Returns a copy of the bitboard, mirrored horizontally.
    @ingroup board
 */
[[nodiscard, gnu::const]] constexpr auto horizontal(Bitboard board) noexcept -> Bitboard;

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

constexpr auto vertical(const Bitboard board) noexcept -> Bitboard
{
    return Bitboard {
        std::byteswap(board.to_int())
    };
}

constexpr auto horizontal(Bitboard board) noexcept -> Bitboard
{
    using namespace literals; // NOLINT

    static constexpr auto k1 = 0x5555555555555555_bb;
    static constexpr auto k2 = 0x3333333333333333_bb;
    static constexpr auto k4 = 0x0f0f0f0f0f0f0f0f_bb;

    board = ((board >> 1uz) & k1) | ((board & k1) << 1uz);
    board = ((board >> 2uz) & k2) | ((board & k2) << 2uz);
    board = ((board >> 4uz) & k4) | ((board & k4) << 4uz);

    return board;
}

} // namespace chess::board::flips
