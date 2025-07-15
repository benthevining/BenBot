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

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/Algebraic.hpp>
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
        const Position startPos {};

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

TEST_CASE("UCI parsing - go", TAGS)
{
    using chess::uci::parse_go_options;

    const Position startPos {};

    SECTION("No arguments")
    {
        const auto opts = parse_go_options("", startPos);

        REQUIRE(opts.moves.empty());
        REQUIRE(not opts.ponderMode);
        REQUIRE(not opts.infinite);
        REQUIRE(not opts.whiteTimeLeft.has_value());
        REQUIRE(not opts.blackTimeLeft.has_value());
        REQUIRE(not opts.whiteInc.has_value());
        REQUIRE(not opts.blackInc.has_value());
        REQUIRE(not opts.movesToGo.has_value());
        REQUIRE(not opts.depth.has_value());
        REQUIRE(not opts.nodes.has_value());
        REQUIRE(not opts.mateIn.has_value());
        REQUIRE(not opts.searchTime.has_value());
    }

    SECTION("Searchmoves")
    {
        using chess::notation::from_alg;

        const auto opts = parse_go_options(
            "  searchmoves  b1c3   g1f3  ponder \n", startPos);

        REQUIRE(opts.moves.size() == 2uz);

        REQUIRE(std::ranges::contains(opts.moves,
            from_alg(startPos, "Nc3")));

        REQUIRE(std::ranges::contains(opts.moves,
            from_alg(startPos, "Nf3")));

        REQUIRE(opts.ponderMode);
    }

    SECTION("Ponder")
    {
        const auto opts = parse_go_options("  ponder ", startPos);

        REQUIRE(opts.ponderMode);
    }

    SECTION("Depth")
    {
        const auto opts = parse_go_options(" depth  8 ", startPos);

        REQUIRE(opts.depth.has_value());
        REQUIRE(*opts.depth == 8uz);
    }
}
