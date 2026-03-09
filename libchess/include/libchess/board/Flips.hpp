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
    using namespace literals; // NOLINT(build/namespaces_literals)

    static constexpr auto msk1 = 0x5555555555555555_bb;
    static constexpr auto msk2 = 0x3333333333333333_bb;
    static constexpr auto msk4 = 0x0f0f0f0f0f0f0f0f_bb;

    board = ((board >> 1uz) & msk1) | ((board & msk1) << 1uz);
    board = ((board >> 2uz) & msk2) | ((board & msk2) << 2uz);
    board = ((board >> 4uz) & msk4) | ((board & msk4) << 4uz);

    return board;
}

} // namespace chess::board::flips
