/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides some bitboard shift algorithms.
    @ingroup board
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>

/** This namespace contains bitboard shifting algorithms.
    @ingroup board
 */
namespace chess::board::shifts {

/// @ingroup board
/// @{

/** Shifts all bits in the given board north (i.e., up one rank). */
[[nodiscard, gnu::const]] constexpr Bitboard north(Bitboard board) noexcept;

/** Shifts all bits in the given board south (i.e., down one rank). */
[[nodiscard, gnu::const]] constexpr Bitboard south(Bitboard board) noexcept;

/** Shifts all bits in the given board east (i.e., right one file). */
[[nodiscard, gnu::const]] constexpr Bitboard east(Bitboard board) noexcept;

/** Shifts all bits in the given board west (i.e., left one file). */
[[nodiscard, gnu::const]] constexpr Bitboard west(Bitboard board) noexcept;

/** Shifts all bits in the given board northeast. */
[[nodiscard, gnu::const]] constexpr Bitboard northeast(Bitboard board) noexcept;

/** Shifts all bits in the given board northwest. */
[[nodiscard, gnu::const]] constexpr Bitboard northwest(Bitboard board) noexcept;

/** Shifts all bits in the given board southeast. */
[[nodiscard, gnu::const]] constexpr Bitboard southeast(Bitboard board) noexcept;

/** Shifts all bits in the given board southwest. */
[[nodiscard, gnu::const]] constexpr Bitboard southwest(Bitboard board) noexcept;

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

constexpr Bitboard north(const Bitboard board) noexcept
{
    return board << 8uz;
}

constexpr Bitboard south(const Bitboard board) noexcept
{
    return board >> 8uz;
}

constexpr Bitboard east(const Bitboard board) noexcept
{
    static constexpr auto notHFile = masks::files::H.inverse();

    return (board & notHFile) << 1uz;
}

constexpr Bitboard west(const Bitboard board) noexcept
{
    static constexpr auto notAFile = masks::files::A.inverse();

    return (board & notAFile) >> 1uz;
}

constexpr Bitboard northeast(const Bitboard board) noexcept
{
    static constexpr auto notHFile = masks::files::H.inverse();

    return (board & notHFile) << 9uz;
}

constexpr Bitboard northwest(const Bitboard board) noexcept
{
    static constexpr auto notAFile = masks::files::A.inverse();

    return (board & notAFile) << 7uz;
}

constexpr Bitboard southeast(const Bitboard board) noexcept
{
    static constexpr auto notHFile = masks::files::H.inverse();

    return (board & notHFile) >> 7uz;
}

constexpr Bitboard southwest(const Bitboard board) noexcept
{
    static constexpr auto notAFile = masks::files::A.inverse();

    return (board & notAFile) >> 9uz;
}

} // namespace chess::board::shifts
