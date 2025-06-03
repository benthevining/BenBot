/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/uci/CommandParsing.hpp>

static constexpr auto TAGS { "[uci][command-parsing]" };

using chess::game::Position;

using chess::notation::from_fen;

TEST_CASE("UCI parsing - position", TAGS)
{
    using chess::uci::parse_position_options;

    SECTION("From start position")
    {
        static constexpr Position startPos {};

        REQUIRE(parse_position_options("  startpos  \n") == startPos);

        REQUIRE(parse_position_options(
                    "fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ")
                == startPos);

        const auto position = parse_position_options("startpos   moves  e2e4  e7e5        g1f3\n");

        REQUIRE(position == from_fen("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"));
    }

    SECTION("From starting FEN")
    {
        const auto position = parse_position_options(
            " fen   5r2/8/1b2k3/8/1P5p/3Q4/2K5/8 b - - 6 7 moves    f8f2 c2c3 e6e5  c3c4   \n");

        REQUIRE(position == from_fen("8/8/1b6/4k3/1PK4p/3Q4/5r2/8 b - - 10 9"));
    }
}
