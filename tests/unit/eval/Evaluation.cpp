/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/notation/FEN.hpp>
#include <limits>

static constexpr auto TAGS { "[eval][static-evaluation]" };

using chess::eval::evaluate;
using chess::notation::from_fen;

namespace match = Catch::Matchers;

TEST_CASE("Evaluation - stalemate", TAGS)
{
    static constexpr auto epsilon = std::numeric_limits<double>::epsilon();

    SECTION("White is stalemated")
    {
        const auto position = from_fen("7K/5k2/6q1/8/8/8/8/8 w - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(0., epsilon));
    }

    SECTION("Black is stalemated")
    {
        const auto position = from_fen("2k5/P7/2K5/6B1/8/8/8/8 b - - 0 1");

        REQUIRE_THAT(
            evaluate(position),
            match::WithinAbs(0., epsilon));
    }
}
