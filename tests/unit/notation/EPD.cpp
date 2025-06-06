/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/EPD.hpp>
#include <string>

static constexpr auto TAGS { "[notation][EPD]" };

using chess::notation::from_epd;

TEST_CASE("EPD - start position", TAGS)
{
    static constexpr chess::game::Position startPos {};

    const auto epd = from_epd("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - hmvc 0; fmvn 1;");

    REQUIRE(epd.position == startPos);
}

TEST_CASE("EPD - operations", TAGS)
{
    using namespace std::literals::string_literals; // NOLINT

    const auto epd = from_epd(
        R"-(r1bqk2r/p1pp1ppp/2p2n2/8/1b2P3/2N5/PPP2PPP/R1BQKB1R w KQkq - bm Bd3; id "Crafty Test Pos.28"; c0 "DB/GK Philadelphia 1996, Game 5, move 7W (Bd3)";)-");

    REQUIRE(epd.operations.at("bm"s) == "Bd3");
    REQUIRE(epd.operations.at("id"s) == "Crafty Test Pos.28");
    REQUIRE(epd.operations.at("c0"s) == "DB/GK Philadelphia 1996, Game 5, move 7W (Bd3)");
}
