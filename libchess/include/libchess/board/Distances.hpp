/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for calculating various measures of distance
    between squares on the chessboard.
    @ingroup board
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::abs()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Square.hpp>
#include <utility>

namespace chess::board {

/// @ingroup board
/// @{

/** Returns as an integer the distance between the file of the first square
    and the file of the second square.

    The maximum file distance is 7.

    @see rank_distance()
    @relates Square
 */
[[nodiscard, gnu::const]] constexpr BitboardIndex file_distance(
    const Square& first, const Square& second) noexcept;

/** Returns as an integer the distance between the rank of the first square
    and the rank of the second square.

    The maximum rank distance is 7.

    @see file_distance()
    @relates Square
 */
[[nodiscard, gnu::const]] constexpr BitboardIndex rank_distance(
    const Square& first, const Square& second) noexcept;

/** Returns true if two squares are on the same diagonal of the chessboard.
    Two squares are on the same diagonal if their file distance equals their rank distance.

    @relates Square
 */
[[nodiscard, gnu::const]] constexpr bool are_on_same_diagonal(
    const Square& first, const Square& second) noexcept;

/** Returns the Manhattan distance between the two squares.

    Manhattan distance is the sum of the rank distance and file distance between the two squares.
    Therefore, this gives the number of non-diagonal king moves required to travel between the two squares.

    The maximum Manhattan distance (between the ends of the long diagonals) is 14.

    This may also be known as "taxicab" distance.

    @see chebyshev_distance()
    @relates Square
 */
[[nodiscard, gnu::const]] constexpr BitboardIndex manhattan_distance(
    const Square& first, const Square& second) noexcept;

/** Returns the Chebyshev distance between the two squares.

    Chebyshev distance is the number of king moves required to travel between the squares on an
    otherwise-empty board. The difference with Manhattan distance is that diagonal king moves
    are considered.

    The maximum Chebyshev distance is 7.

    @see manhattan_distance()
    @relates Square
 */
[[nodiscard, gnu::const]] constexpr BitboardIndex chebyshev_distance(
    const Square& first, const Square& second) noexcept;

/** Returns the knight distance between the two squares; that is, the number of moves a knight
    requires to maneuver from ``first`` to ``second``.

    @relates Square
 */
[[nodiscard, gnu::const]] inline BitboardIndex knight_distance(
    const Square& first, const Square& second);

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

constexpr BitboardIndex file_distance(
    const Square& first, const Square& second) noexcept
{
    const auto firstFile  = std::to_underlying(first.file);
    const auto secondFile = std::to_underlying(second.file);

    const auto [minFile, maxFile] = std::minmax(firstFile, secondFile);

    return maxFile - minFile;
}

constexpr BitboardIndex rank_distance(
    const Square& first, const Square& second) noexcept
{
    const auto firstRank  = std::to_underlying(first.rank);
    const auto secondRank = std::to_underlying(second.rank);

    const auto [minRank, maxRank] = std::minmax(firstRank, secondRank);

    return maxRank - minRank;
}

constexpr bool are_on_same_diagonal(
    const Square& first, const Square& second) noexcept
{
    return std::cmp_equal(file_distance(first, second), rank_distance(first, second));
}

constexpr BitboardIndex manhattan_distance(
    const Square& first, const Square& second) noexcept
{
    return file_distance(first, second) + rank_distance(first, second);
}

constexpr BitboardIndex chebyshev_distance(
    const Square& first, const Square& second) noexcept
{
    const auto fileDist = file_distance(first, second);
    const auto rankDist = rank_distance(first, second);

    return std::max(fileDist, rankDist);
}

inline BitboardIndex knight_distance(
    const Square& first, const Square& second)
{
    using std::size_t;

    static constexpr std::array ndis {
        0, 3, 2, 3, 2, 3, 4, 5,
        3, 2, 1, 2, 3, 4, 3, 4,
        2, 1, 4, 3, 2, 3, 4, 5,
        3, 2, 3, 2, 3, 4, 3, 4,
        2, 3, 2, 3, 4, 3, 4, 5,
        3, 4, 3, 4, 3, 4, 5, 4,
        4, 3, 4, 3, 4, 5, 4, 5,
        5, 4, 5, 4, 5, 4, 5, 6
    };

    static constexpr std::array corner {
        1, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 1
    };

    const auto firstIdx  = static_cast<int>(first.index());
    const auto secondIdx = static_cast<int>(second.index());

    // NB. this isn't the same as manhattan_distance()
    const auto absDist = [firstIdx, secondIdx] {
        const auto rankDist = (firstIdx | 7) - (secondIdx | 7);
        const auto fileDist = (firstIdx & 7) - (secondIdx & 7);

        return std::abs(rankDist) + std::abs(fileDist);
    }();

    auto dist = ndis.at(static_cast<size_t>(absDist));

    if (std::cmp_equal(absDist, 9)) {
        [[unlikely]];

        const auto cornerMask = corner.at(static_cast<size_t>(firstIdx))
                              ^ corner.at(static_cast<size_t>(secondIdx));

        dist += 2 * cornerMask;
    }

    assert(dist >= 0);
    assert(dist <= 6);

    return static_cast<BitboardIndex>(dist);
}

} // namespace chess::board
