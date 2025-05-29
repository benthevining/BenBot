/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>

static constexpr auto TAGS { "[moves][Generation][legal]" };

using chess::notation::from_fen;

#if 0
TEST_CASE("Maximum known moves in a position", TAGS)
{
    const auto position = from_fen("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1");

    const auto moves = chess::moves::generate(position);

    REQUIRE(moves.size() == 218uz);
}
#endif
