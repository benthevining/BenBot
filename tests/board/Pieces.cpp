/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/pieces/Colors.hpp>

static constexpr auto TAGS { "[board][Pieces]" };

using chess::board::Pieces;
using chess::board::Rank;
using chess::pieces::Color;

TEST_CASE("Pieces - starting position", TAGS)
{
    SECTION("White")
    {
        static constexpr Pieces pieces { Color::White };

        STATIC_REQUIRE(pieces.material() == 41uz);

        static constexpr auto occupied = pieces.occupied();

        STATIC_REQUIRE(occupied.count() == 16uz);

        for (const auto square : occupied.squares())
            REQUIRE(((square.rank == Rank::One) || (square.rank == Rank::Two)));
    }

    SECTION("Black")
    {
        static constexpr Pieces pieces { Color::Black };

        STATIC_REQUIRE(pieces.material() == 41uz);

        static constexpr auto occupied = pieces.occupied();

        STATIC_REQUIRE(occupied.count() == 16uz);

        for (const auto square : occupied.squares())
            REQUIRE(((square.rank == Rank::Seven) || (square.rank == Rank::Eight)));
    }
}
