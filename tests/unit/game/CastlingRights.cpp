/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/game/CastlingRights.hpp>

static constexpr auto TAGS { "[game][CastlingRights]" };

using chess::game::CastlingRights;

TEST_CASE("Castling rights - default", TAGS)
{
    static constexpr CastlingRights rights;

    STATIC_REQUIRE(rights.either());
    STATIC_REQUIRE(rights.kingside);
    STATIC_REQUIRE(rights.queenside);
}

TEST_CASE("Castling rights - king_moved()", TAGS)
{
    CastlingRights rights;

    REQUIRE(rights.either());

    rights.king_moved();

    REQUIRE(! rights.either());
}

TEST_CASE("Castling rights - rook_moved()", TAGS)
{
    CastlingRights rights;

    REQUIRE(rights.either());

    SECTION("Kingside")
    {
        rights.rook_moved(true);

        REQUIRE(! rights.kingside);
        REQUIRE(rights.either());
    }

    SECTION("Queenside")
    {
        rights.rook_moved(false);

        REQUIRE(! rights.queenside);
        REQUIRE(rights.either());
    }
}
