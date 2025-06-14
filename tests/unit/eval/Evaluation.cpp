/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libchess/notation/FEN.hpp>
#include <limits>

static constexpr auto TAGS { "[eval][static-evaluation]" };

using chess::eval::evaluate;
using chess::notation::from_fen;

namespace match = Catch::Matchers;

static constexpr auto epsilon = std::numeric_limits<double>::epsilon();

TEST_CASE("Evaluation - stalemate", TAGS)
{
    SECTION("White is stalemated")
    {
        const auto position = from_fen("7K/5k2/6q1/8/8/8/8/8 w - - 0 1");

        REQUIRE(position.is_stalemate());

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }

    SECTION("Black is stalemated")
    {
        const auto position = from_fen("2k5/P7/2K5/6B1/8/8/8/8 b - - 0 1");

        REQUIRE(position.is_stalemate());

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }
}

TEST_CASE("Evaluation - checkmate", TAGS)
{
    SECTION("White is checkmated")
    {
        const auto position = from_fen("rnb1kb1r/pppppppp/4q3/8/2P5/1B1n4/PP1PPPPP/RN1QKBNR w KQkq - 0 1");

        REQUIRE(position.is_checkmate());

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(-chess::eval::MATE, epsilon));
    }

    SECTION("Black is checkmated")
    {
        const auto position = from_fen("r2qkbnr/ppp1pBpp/2n5/1b1pN3/8/4PQ2/PPPP1PPP/R1B1K1NR b KQkq - 0 1");

        REQUIRE(position.is_checkmate());

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(-chess::eval::MATE, epsilon));
    }
}

TEST_CASE("Evaluation - draw by insufficient material", TAGS)
{
    SECTION("Lone kings")
    {
        const auto position = from_fen("8/8/1K6/8/5k2/8/8/8 w - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }

    SECTION("White has a single knight")
    {
        const auto position = from_fen("8/8/1K6/8/5k2/3N4/8/8 b - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }

    SECTION("White has a single bishop")
    {
        const auto position = from_fen("8/8/1K6/8/5k2/8/8/5B2 b - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }

    SECTION("Black has a single knight")
    {
        const auto position = from_fen("8/8/1K1n4/8/5k2/8/8/8 w - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }

    SECTION("Black has a single bishop")
    {
        const auto position = from_fen("8/8/1K6/8/3b1k2/8/8/8 w - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(chess::eval::DRAW, epsilon));
    }
}
