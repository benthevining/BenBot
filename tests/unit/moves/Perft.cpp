/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/moves/MoveGen.hpp>

using chess::moves::perft;

TEST_CASE("Move gen - perft", "[moves][Generation][perft]")
{
    STATIC_REQUIRE(perft(0uz) == 1uz);
    STATIC_REQUIRE(perft(1uz) == 20uz);

    // on Clang this already hits the constexpr recursion limits but works at runtime
    REQUIRE(perft(2uz) == 400uz);
    REQUIRE(perft(3uz) == 8902uz);
}
