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

using chess::moves::generate;
using chess::notation::from_fen;

TEST_CASE("Maximum known moves in a position", TAGS)
{
    const auto position = from_fen("R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1");

    const auto moves = generate(position);

    REQUIRE(moves.size() == 218uz);
}

TEST_CASE("Captures only", TAGS)
{
    SECTION("No captures")
    {
        const auto position = from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

        REQUIRE(generate<true>(position).empty());
    }

    SECTION("Kiwipete")
    {
        const auto position = from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

        REQUIRE(generate<true>(position).size() == 8uz);
    }

    SECTION("Rook/pawn endgame")
    {
        const auto position = from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");

        REQUIRE(generate<true>(position).size() == 1uz);
    }
}
