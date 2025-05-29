/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/moves/Perft.hpp>

static constexpr auto TAGS { "[moves][Generation][perft]" };

using chess::moves::perft;

TEST_CASE("Perft - depth 0", TAGS)
{
    static constexpr auto result = perft(0uz);

    STATIC_REQUIRE(result.nodes == 1uz);
    STATIC_REQUIRE(result.captures == 0uz);
    STATIC_REQUIRE(result.enPassantCaptures == 0uz);
    STATIC_REQUIRE(result.castles == 0uz);
    STATIC_REQUIRE(result.promotions == 0uz);
    STATIC_REQUIRE(result.checks == 0uz);
    STATIC_REQUIRE(result.checkmates == 0uz);
    STATIC_REQUIRE(result.stalemates == 0uz);
}

TEST_CASE("Perft - depth 1", TAGS)
{
    const auto result = perft(1uz);

    REQUIRE(result.nodes == 20uz);
    REQUIRE(result.captures == 0uz);
    REQUIRE(result.enPassantCaptures == 0uz);
    REQUIRE(result.castles == 0uz);
    REQUIRE(result.promotions == 0uz);
    REQUIRE(result.checks == 0uz);
    REQUIRE(result.checkmates == 0uz);
    REQUIRE(result.stalemates == 0uz);
}

TEST_CASE("Perft - depth 2", TAGS)
{
    const auto result = perft(2uz);

    REQUIRE(result.nodes == 400uz);
    REQUIRE(result.captures == 0uz);
    REQUIRE(result.enPassantCaptures == 0uz);
    REQUIRE(result.castles == 0uz);
    REQUIRE(result.promotions == 0uz);
    REQUIRE(result.checks == 0uz);
    REQUIRE(result.checkmates == 0uz);
    REQUIRE(result.stalemates == 0uz);
}

TEST_CASE("Perft - depth 3", TAGS)
{
    const auto result = perft(3uz);

    REQUIRE(result.nodes == 8902uz);
    REQUIRE(result.captures == 34uz);
    REQUIRE(result.enPassantCaptures == 0uz);
    REQUIRE(result.castles == 0uz);
    REQUIRE(result.promotions == 0uz);
    CHECK(result.checks == 12uz);
    CHECK(result.checkmates == 0uz);
    REQUIRE(result.stalemates == 0uz);
}
