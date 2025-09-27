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

#include <catch2/catch_test_macros.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/EPD.hpp>
#include <string>

static constexpr auto TAGS { "[notation][EPD]" };

using chess::notation::from_epd;

TEST_CASE("EPD - start position", TAGS)
{
    const chess::game::Position startPos {};

    const auto epd = from_epd("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - hmvc 0; fmvn 1;")
                         .value();

    REQUIRE(epd.position == startPos);
}

TEST_CASE("EPD - operations", TAGS)
{
    using namespace std::literals::string_literals; // NOLINT

    const auto epd = from_epd(
        R"-(r1bqk2r/p1pp1ppp/2p2n2/8/1b2P3/2N5/PPP2PPP/R1BQKB1R w KQkq - bm Bd3; id "Crafty Test Pos.28"; c0 "DB/GK Philadelphia 1996, Game 5, move 7W (Bd3)";)-")
                         .value();

    REQUIRE(epd.operations.at("bm"s) == "Bd3");
    REQUIRE(epd.operations.at("id"s) == "Crafty Test Pos.28");
    REQUIRE(epd.operations.at("c0"s) == "DB/GK Philadelphia 1996, Game 5, move 7W (Bd3)");
}

TEST_CASE("EPD - round tripping", TAGS)
{
    using namespace std::literals::string_literals; // NOLINT

    // note that fmvn & hmvc operations will be added by to_epd() by default if not present in the input EPD
    const auto epd1 = from_epd(
        R"-(r1bqk2r/p1pp1ppp/2p2n2/8/1b2P3/2N5/PPP2PPP/R1BQKB1R w KQkq - bm Bd3; id "Crafty Test Pos.28"; c0 "DB/GK Philadelphia 1996, Game 5, move 7W (Bd3)"; fmvn 1; hmvc 1)-")
                          .value();

    const auto epd2 = from_epd(to_epd(epd1));

    REQUIRE(epd1 == epd2);
}

TEST_CASE("EPD - parse_all_epds()", TAGS)
{
    using namespace std::literals::string_literals; // NOLINT

    static constexpr auto epdText =
        R"-(r1bqk2r/p1pp1ppp/2p2n2/8/1b2P3/2N5/PPP2PPP/R1BQKB1R w KQkq - bm Bd3; id "Crafty Test Pos.28"; c0 "DB/GK Philadelphia 1996, Game 5, move 7W (Bd3)";
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - hmvc 0; fmvn 1;
1r4k1/4ppb1/2n1b1qp/pB4p1/1n1BP1P1/7P/2PNQPK1/3RN3 w - -)-";

    const auto epds = chess::notation::parse_all_epds(epdText);

    REQUIRE(epds.size() == 3uz);
}
