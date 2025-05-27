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

    [[nodiscard, gnu::const]] constexpr Bitboard ray_north(const Square& starting) noexcept
    {
        static constexpr Bitboard mask { 0x0101010101010100 };

        return mask << starting.index();
    }

    [[nodiscard, gnu::const]] constexpr Bitboard ray_south(const Square& starting) noexcept
    {
        static constexpr Bitboard mask { 0x0080808080808080 };

        return mask >> (starting.index() ^ 63);
    }

    [[nodiscard, gnu::const]] constexpr Bitboard ray_east(const Square& starting) noexcept
    {
        static constexpr Bitboard::Integer one { 1 };

        const auto index = starting.index();

        return Bitboard {
            2 * ((one << (index | 7)) - (one << index))
        };
    }

    [[nodiscard, gnu::const]] constexpr Bitboard ray_west(const Square& starting) noexcept
    {
        static constexpr Bitboard::Integer one { 1 };

        const auto index = starting.index();

        return Bitboard {
            (one << index) - (one << (index & 56))
        };
    }

    [[nodiscard, gnu::const]] constexpr Bitboard north_ray_attacks(const Square& starting, const Bitboard occupied) noexcept
    {
        auto attacks = ray_north(starting);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = blocker.first();
            attacks ^= ray_north(Square::from_index(idx));
        }

        return attacks;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard east_ray_attacks(const Square& starting, const Bitboard occupied) noexcept
    {
        auto attacks = ray_east(starting);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = blocker.first();
            attacks ^= ray_east(Square::from_index(idx));
        }

        return attacks;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard south_ray_attacks(const Square& starting, const Bitboard occupied) noexcept
    {
        auto attacks = ray_south(starting);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = blocker.last();
            attacks ^= ray_south(Square::from_index(idx));
        }

        return attacks;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard west_ray_attacks(const Square& starting, const Bitboard occupied) noexcept
    {
        auto attacks = ray_west(starting);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = blocker.last();
            attacks ^= ray_west(Square::from_index(idx));
        }

        return attacks;
    }

} // namespace detail

} // namespace chess::moves::legal
