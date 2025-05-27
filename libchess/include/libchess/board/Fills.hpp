/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides some bitboard fill algorithms.
    @ingroup board
 */

#pragma once

#include <libchess/board/Bitboard.hpp>

/** This namespace contains bitboard fill algorithms.
    @ingroup board
 */
namespace chess::board::fills {

/// @ingroup board
/// @{

/** Performs a north fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard north(Bitboard starting) noexcept;

/** Performs a south fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard south(Bitboard starting) noexcept;

/** Performs a file fill of the starting bitboard.
    For any file in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that file set to 1.
 */
[[nodiscard, gnu::const]] constexpr Bitboard file(Bitboard starting) noexcept;

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

constexpr Bitboard north(Bitboard starting) noexcept
{
    starting |= (starting << 8uz);
    starting |= (starting << 16uz);
    starting |= (starting << 32uz);

    return starting;
}

constexpr Bitboard south(Bitboard starting) noexcept
{
    starting |= (starting >> 8uz);
    starting |= (starting >> 16uz);
    starting |= (starting >> 32uz);

    return starting;
}

constexpr Bitboard file(const Bitboard starting) noexcept
{
    return north(starting) | south(starting);
}

} // namespace chess::board::fills
