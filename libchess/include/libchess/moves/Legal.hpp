/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Square.hpp>

namespace chess::moves::legal {

using board::Bitboard;
using board::Square;

// pawn pushes
// pawn double pushes
// pawn captures
// knight
// bishop
// queen
// king

/** Calculates all legal rook moves, taking blocking pieces into consideration.
    @todo prune squares occupied by own pieces
 */
[[nodiscard, gnu::const]] constexpr Bitboard rook(const Square& starting, Bitboard occupied) noexcept;

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

namespace detail {

    enum class RayDirection {
        North,
        East,
        South,
        West
    };

    [[nodiscard, gnu::const]] constexpr bool is_negative(const RayDirection direction) noexcept
    {
        return std::cmp_greater_equal(
            std::to_underlying(direction),
            std::to_underlying(RayDirection::South));
    }

    [[nodiscard, gnu::const]] constexpr Bitboard make_ray(
        const board::BitboardIndex startPos, const RayDirection direction) noexcept
    {
        static constexpr Bitboard::Integer ONE { 1 };

        switch (direction) {
            case RayDirection::North: {
                static constexpr Bitboard mask { 0x0101010101010100 };

                return mask << startPos;
            }

            case RayDirection::South: {
                static constexpr Bitboard mask { 0x0080808080808080 };

                return mask >> (startPos ^ 63);
            }

            case RayDirection::East: {
                return Bitboard {
                    2 * ((ONE << (startPos | 7)) - (ONE << startPos))
                };
            }

            case RayDirection::West: {
                return Bitboard {
                    (ONE << startPos) - (ONE << (startPos & 56))
                };
            }

            default:
                std::unreachable();
        }
    }

    // returns all squares accessible by a ray attacker in the given direction,
    // stopping at the first blocking piece as indicated by the occupied bitboard
    [[nodiscard, gnu::const]] constexpr Bitboard ray_attacks(
        const board::BitboardIndex startPos, const RayDirection direction, const Bitboard occupied) noexcept
    {
        auto attacks = make_ray(startPos, direction);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = is_negative(direction) ? blocker.last() : blocker.first();

            attacks ^= make_ray(idx, direction);
        }

        return attacks;
    }

} // namespace detail

constexpr Bitboard rook(
    const Square& starting, const Bitboard occupied) noexcept
{
    using detail::ray_attacks;
    using detail::RayDirection;

    const auto startPos = starting.index();

    return ray_attacks(startPos, RayDirection::North, occupied)
         | ray_attacks(startPos, RayDirection::East, occupied)
         | ray_attacks(startPos, RayDirection::South, occupied)
         | ray_attacks(startPos, RayDirection::West, occupied);
}

} // namespace chess::moves::legal
