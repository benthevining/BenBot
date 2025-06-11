/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <array>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::abs()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/board/Distances.hpp>
#include <utility>

namespace chess::board {

using std::size_t;

BitboardIndex knight_distance(
    const Square& first, const Square& second)
{
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

    const auto firstIdx  = first.index();
    const auto secondIdx = second.index();

    // NB. this isn't the same as manhattan_distance()
    const auto absDist = [firstIdx, secondIdx] {
        const auto rankDist = static_cast<int>(firstIdx | 7uz) - static_cast<int>(secondIdx | 7uz);
        const auto fileDist = static_cast<int>(firstIdx & 7uz) - static_cast<int>(secondIdx & 7uz);

        return std::abs(rankDist) + std::abs(fileDist);
    }();

    assert(absDist >= 0);

    auto dist = static_cast<size_t>(ndis.at(static_cast<size_t>(absDist)));

    if (std::cmp_equal(absDist, 9)) {
        [[unlikely]];

        const auto firstMask  = static_cast<size_t>(corner.at(firstIdx));
        const auto secondMask = static_cast<size_t>(corner.at(secondIdx));

        dist += 2uz * (firstMask ^ secondMask);
    }

    assert(dist <= 6uz);

    return static_cast<BitboardIndex>(dist);
}

} // namespace chess::board
