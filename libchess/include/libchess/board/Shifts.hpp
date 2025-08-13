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
    This file provides some bitboard shift algorithms.
    @ingroup board
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace contains bitboard shifting algorithms.
    @ingroup board
 */
namespace chess::board::shifts {

using pieces::Color;

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

/** Shifts all pawn positions in the starting bitboard forward, from the given side's perspective. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_forward(Bitboard board) noexcept;

/** Shifts all pawn positions in the starting bitboard backwards, from the given side's perspective. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_backward(Bitboard board) noexcept;

/** Shifts all bits in the given bitboard to the east capturing direction for the given side. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_capture_east(Bitboard board) noexcept;

/** Shifts all bits in the given bitboard to the west capturing direction for the given side. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_capture_west(Bitboard board) noexcept;

/** The inverse operation of ``pawn_capture_east()``.
    Given a set of target squares, this function returns the set of squares that pawns must
    start from in order to reach the target squares by capturing east.
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_inv_capture_east(Bitboard board) noexcept;

/** The inverse operation of ``pawn_capture_west()``.
    Given a set of target squares, this function returns the set of squares that pawns must
    start from in order to reach the target squares by capturing west.
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_inv_capture_west(Bitboard board) noexcept;

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
    constexpr auto notHFile = masks::files::H.inverse();

    return (board & notHFile) << 1uz;
}

constexpr Bitboard west(const Bitboard board) noexcept
{
    constexpr auto notAFile = masks::files::A.inverse();

    return (board & notAFile) >> 1uz;
}

constexpr Bitboard northeast(const Bitboard board) noexcept
{
    constexpr auto notHFile = masks::files::H.inverse();

    return (board & notHFile) << 9uz;
}

constexpr Bitboard northwest(const Bitboard board) noexcept
{
    constexpr auto notAFile = masks::files::A.inverse();

    return (board & notAFile) << 7uz;
}

constexpr Bitboard southeast(const Bitboard board) noexcept
{
    constexpr auto notHFile = masks::files::H.inverse();

    return (board & notHFile) >> 7uz;
}

constexpr Bitboard southwest(const Bitboard board) noexcept
{
    constexpr auto notAFile = masks::files::A.inverse();

    return (board & notAFile) >> 9uz;
}

template <Color Side>
constexpr Bitboard pawn_forward(const Bitboard board) noexcept
{
    if constexpr (Side == Color::White)
        return north(board);
    else
        return south(board);
}

template <Color Side>
constexpr Bitboard pawn_backward(const Bitboard board) noexcept
{
    if constexpr (Side == Color::White)
        return south(board);
    else
        return north(board);
}

template <Color Side>
constexpr Bitboard pawn_capture_east(const Bitboard board) noexcept
{
    if constexpr (Side == Color::White)
        return northeast(board);
    else
        return southeast(board);
}

template <Color Side>
constexpr Bitboard pawn_inv_capture_east(const Bitboard board) noexcept
{
    if constexpr (Side == Color::White)
        return southwest(board);
    else
        return northwest(board);
}

template <Color Side>
constexpr Bitboard pawn_capture_west(const Bitboard board) noexcept
{
    if constexpr (Side == Color::White)
        return northwest(board);
    else
        return southwest(board);
}

template <Color Side>
constexpr Bitboard pawn_inv_capture_west(const Bitboard board) noexcept
{
    if constexpr (Side == Color::White)
        return southeast(board);
    else
        return northeast(board);
}

} // namespace chess::board::shifts
