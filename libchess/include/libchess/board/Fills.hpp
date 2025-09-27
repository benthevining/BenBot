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
    This file provides some bitboard fill algorithms.
    @ingroup board
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace contains bitboard fill algorithms.
    @ingroup board
 */
namespace chess::board::fills {

using pieces::Color;

/// @ingroup board
/// @{

/** Performs a north fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto north(Bitboard starting) noexcept -> Bitboard;

/** Performs a south fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto south(Bitboard starting) noexcept -> Bitboard;

/** Performs an east fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto east(Bitboard starting) noexcept -> Bitboard;

/** Performs a west fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto west(Bitboard starting) noexcept -> Bitboard;

/** Performs a northeast fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto northeast(Bitboard starting) noexcept -> Bitboard;

/** Performs a southeast fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto southeast(Bitboard starting) noexcept -> Bitboard;

/** Performs a northwest fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto northwest(Bitboard starting) noexcept -> Bitboard;

/** Performs a southwest fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr auto southwest(Bitboard starting) noexcept -> Bitboard;

/** Performs a file fill of the starting bitboard.
    For any file in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that file set to 1.
 */
[[nodiscard, gnu::const]] constexpr auto file(Bitboard starting) noexcept -> Bitboard;

/** Performs a rank fill of the starting bitboard.
    For any rank in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that rank set to 1.
 */
[[nodiscard, gnu::const]] constexpr auto rank(Bitboard starting) noexcept -> Bitboard;

/** Performs a diagonal fill of the starting bitboard.
    For any diagonal in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that diagonal set to 1.
 */
[[nodiscard, gnu::const]] constexpr auto diagonal(Bitboard starting) noexcept -> Bitboard;

/** Performs an antidiagonal fill of the starting bitboard.
    For any antidiagonal in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that antidiagonal set to 1.
 */
[[nodiscard, gnu::const]] constexpr auto antidiagonal(Bitboard starting) noexcept -> Bitboard;

/** Performs a pawn front-fill from the given starting position. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto pawn_front(Bitboard starting) noexcept -> Bitboard;

/** Performs a pawn rear-fill from the given starting position. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto pawn_rear(Bitboard starting) noexcept -> Bitboard;

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

constexpr auto north(Bitboard starting) noexcept -> Bitboard
{
    starting |= (starting << 8uz);
    starting |= (starting << 16uz);
    starting |= (starting << 32uz);

    return starting;
}

constexpr auto south(Bitboard starting) noexcept -> Bitboard
{
    starting |= (starting >> 8uz);
    starting |= (starting >> 16uz);
    starting |= (starting >> 32uz);

    return starting;
}

constexpr auto east(Bitboard starting) noexcept -> Bitboard
{
    constexpr auto notAFile = masks::files::A.inverse();

    constexpr auto mask1 = notAFile & (notAFile << 1uz);
    constexpr auto mask2 = mask1 & (mask1 << 2uz);

    starting |= notAFile & (starting << 1uz);
    starting |= mask1 & (starting << 2uz);
    starting |= mask2 & (starting << 4uz);

    return starting;
}

constexpr auto west(Bitboard starting) noexcept -> Bitboard
{
    constexpr auto notHFile = masks::files::H.inverse();

    constexpr auto mask1 = notHFile & (notHFile >> 1uz);
    constexpr auto mask2 = mask1 & (mask1 >> 2uz);

    starting |= notHFile & (starting >> 1uz);
    starting |= mask1 & (starting >> 2uz);
    starting |= mask2 & (starting >> 4uz);

    return starting;
}

constexpr auto northeast(Bitboard starting) noexcept -> Bitboard
{
    constexpr auto notAFile = masks::files::A.inverse();

    constexpr auto mask1 = notAFile & (notAFile << 9uz);
    constexpr auto mask2 = mask1 & (mask1 << 18uz);

    starting |= notAFile & (starting << 9uz);
    starting |= mask1 & (starting << 18uz);
    starting |= mask2 & (starting << 36uz);

    return starting;
}

constexpr auto southeast(Bitboard starting) noexcept -> Bitboard
{
    constexpr auto notAFile = masks::files::A.inverse();

    constexpr auto mask1 = notAFile & (notAFile >> 7uz);
    constexpr auto mask2 = mask1 & (mask1 >> 14uz);

    starting |= notAFile & (starting >> 7uz);
    starting |= mask1 & (starting >> 14uz);
    starting |= mask2 & (starting >> 28uz);

    return starting;
}

constexpr auto northwest(Bitboard starting) noexcept -> Bitboard
{
    constexpr auto notHFile = masks::files::H.inverse();

    constexpr auto mask1 = notHFile & (notHFile << 7uz);
    constexpr auto mask2 = mask1 & (mask1 << 14uz);

    starting |= notHFile & (starting << 7uz);
    starting |= mask1 & (starting << 14uz);
    starting |= mask2 & (starting << 28uz);

    return starting;
}

constexpr auto southwest(Bitboard starting) noexcept -> Bitboard
{
    constexpr auto notHFile = masks::files::H.inverse();

    constexpr auto mask1 = notHFile & (notHFile >> 9uz);
    constexpr auto mask2 = mask1 & (mask1 >> 18uz);

    starting |= notHFile & (starting >> 9uz);
    starting |= mask1 & (starting >> 18uz);
    starting |= mask2 & (starting >> 36uz);

    return starting;
}

constexpr auto file(const Bitboard starting) noexcept -> Bitboard
{
    return north(starting) | south(starting);
}

constexpr auto rank(const Bitboard starting) noexcept -> Bitboard
{
    return east(starting) | west(starting);
}

constexpr auto diagonal(const Bitboard starting) noexcept -> Bitboard
{
    return northeast(starting) | southwest(starting);
}

constexpr auto antidiagonal(const Bitboard starting) noexcept -> Bitboard
{
    return northwest(starting) | southeast(starting);
}

template <Color Side>
constexpr auto pawn_front(const Bitboard starting) noexcept -> Bitboard
{
    if constexpr (Side == Color::White)
        return north(starting);
    else
        return south(starting);
}

template <Color Side>
constexpr auto pawn_rear(const Bitboard starting) noexcept -> Bitboard
{
    if constexpr (Side == Color::White)
        return south(starting);
    else
        return north(starting);
}

} // namespace chess::board::fills
