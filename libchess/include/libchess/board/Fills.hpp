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
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace contains bitboard fill algorithms.
    @ingroup board
 */
namespace chess::board::fills {

using pieces::Color;

/// @ingroup board
/// @{

/** Performs a north fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard north(Bitboard starting) noexcept;

/** Performs a south fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard south(Bitboard starting) noexcept;

/** Performs an east fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard east(Bitboard starting) noexcept;

/** Performs a west fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard west(Bitboard starting) noexcept;

/** Performs a northeast fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard northeast(Bitboard starting) noexcept;

/** Performs a southeast fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard southeast(Bitboard starting) noexcept;

/** Performs a northwest fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard northwest(Bitboard starting) noexcept;

/** Performs a southwest fill of the starting bitboard. */
[[nodiscard, gnu::const]] constexpr Bitboard southwest(Bitboard starting) noexcept;

/** Performs a file fill of the starting bitboard.
    For any file in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that file set to 1.
 */
[[nodiscard, gnu::const]] constexpr Bitboard file(Bitboard starting) noexcept;

/** Performs a rank fill of the starting bitboard.
    For any rank in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that rank set to 1.
 */
[[nodiscard, gnu::const]] constexpr Bitboard rank(Bitboard starting) noexcept;

/** Performs a diagonal fill of the starting bitboard.
    For any diagonal in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that diagonal set to 1.
 */
[[nodiscard, gnu::const]] constexpr Bitboard diagonal(Bitboard starting) noexcept;

/** Performs an antidiagonal fill of the starting bitboard.
    For any antidiagonal in the starting bitboard with at least 1 bit set, the returned
    bitboard will have all bits on that antidiagonal set to 1.
 */
[[nodiscard, gnu::const]] constexpr Bitboard antidiagonal(Bitboard starting) noexcept;

/** Performs a pawn front-fill from the given starting position. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_front(Bitboard starting) noexcept;

/** Performs a pawn rear-fill from the given starting position. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_rear(Bitboard starting) noexcept;

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

constexpr Bitboard east(Bitboard starting) noexcept
{
    static constexpr auto notAFile = masks::files::A.inverse();

    static constexpr auto mask1 = notAFile & (notAFile << 1uz);
    static constexpr auto mask2 = mask1 & (mask1 << 2uz);

    starting |= notAFile & (starting << 1uz);
    starting |= mask1 & (starting << 2uz);
    starting |= mask2 & (starting << 4uz);

    return starting;
}

constexpr Bitboard west(Bitboard starting) noexcept
{
    static constexpr auto notHFile = masks::files::H.inverse();

    static constexpr auto mask1 = notHFile & (notHFile >> 1uz);
    static constexpr auto mask2 = mask1 & (mask1 >> 2uz);

    starting |= notHFile & (starting >> 1uz);
    starting |= mask1 & (starting >> 2uz);
    starting |= mask2 & (starting >> 4uz);

    return starting;
}

constexpr Bitboard northeast(Bitboard starting) noexcept
{
    static constexpr auto notAFile = masks::files::A.inverse();

    static constexpr auto mask1 = notAFile & (notAFile << 9uz);
    static constexpr auto mask2 = mask1 & (mask1 << 18uz);

    starting |= notAFile & (starting << 9uz);
    starting |= mask1 & (starting << 18uz);
    starting |= mask2 & (starting << 36uz);

    return starting;
}

constexpr Bitboard southeast(Bitboard starting) noexcept
{
    static constexpr auto notAFile = masks::files::A.inverse();

    static constexpr auto mask1 = notAFile & (notAFile >> 7uz);
    static constexpr auto mask2 = mask1 & (mask1 >> 14uz);

    starting |= notAFile & (starting >> 7uz);
    starting |= mask1 & (starting >> 14uz);
    starting |= mask2 & (starting >> 28uz);

    return starting;
}

constexpr Bitboard northwest(Bitboard starting) noexcept
{
    static constexpr auto notHFile = masks::files::H.inverse();

    static constexpr auto mask1 = notHFile & (notHFile << 7uz);
    static constexpr auto mask2 = mask1 & (mask1 << 14uz);

    starting |= notHFile & (starting << 7uz);
    starting |= mask1 & (starting << 14uz);
    starting |= mask2 & (starting << 28uz);

    return starting;
}

constexpr Bitboard southwest(Bitboard starting) noexcept
{
    static constexpr auto notHFile = masks::files::H.inverse();

    static constexpr auto mask1 = notHFile & (notHFile >> 9uz);
    static constexpr auto mask2 = mask1 & (mask1 >> 18uz);

    starting |= notHFile & (starting >> 9uz);
    starting |= mask1 & (starting >> 18uz);
    starting |= mask2 & (starting >> 36uz);

    return starting;
}

constexpr Bitboard file(const Bitboard starting) noexcept
{
    return north(starting) | south(starting);
}

constexpr Bitboard rank(const Bitboard starting) noexcept
{
    return east(starting) | west(starting);
}

constexpr Bitboard diagonal(const Bitboard starting) noexcept
{
    return northeast(starting) | southwest(starting);
}

constexpr Bitboard antidiagonal(Bitboard starting) noexcept
{
    return northwest(starting) | southeast(starting);
}

template <Color Side>
constexpr Bitboard pawn_front(const Bitboard starting) noexcept
{
    if constexpr (Side == Color::White)
        return north(starting);
    else
        return south(starting);
}

template <Color Side>
constexpr Bitboard pawn_rear(const Bitboard starting) noexcept
{
    if constexpr (Side == Color::White)
        return south(starting);
    else
        return north(starting);
}

} // namespace chess::board::fills
