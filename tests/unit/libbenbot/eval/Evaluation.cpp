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
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>

inline constexpr auto TAGS { "[eval][evaluation][!mayfail]" };

using ben_bot::eval::evaluate;
using chess::game::Position;
using chess::notation::from_fen;

TEST_CASE("Evaluation symmetry", TAGS)
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    SECTION("Starting position")
    {
        static const Position startPos;

        REQUIRE(
            evaluate(startPos, pieceSquareTables)
            == evaluate(flipped(startPos), pieceSquareTables));
    }

    SECTION("Kiwipete")
    {
        const auto position = from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Mate in 1")
    {
        const auto position = from_fen("r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Mate")
    {
        const auto position = from_fen("8/8/8/8/8/8/3q4/2K1k3 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Promoting")
    {
        const auto position = from_fen("8/2P5/8/8/k1K5/8/8/8 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Uneven piece mobility")
    {
        const auto position = from_fen("r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Pawn attacking a rook")
    {
        const auto position = from_fen("4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Pawn forking a knight & bishop")
    {
        const auto position = from_fen("r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }
}

TEST_CASE("Evaluation - mobility", "[eval][evaluation][mobility]")
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    SECTION("Bishop blocked in by its own pawns")
    {
        // The only difference is the white bishop on c1 vs f1, which are mirrored squares, so
        // all the other eval terms are the same. The bishop on c1 can't move.
        const auto trapped = from_fen("2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1").value();
        const auto free    = from_fen("2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1").value();

        REQUIRE(evaluate(trapped, pieceSquareTables) < evaluate(free, pieceSquareTables));
    }
}

TEST_CASE("Evaluation - threats", "[eval][evaluation][threats]")
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    // The black knight is on e5 (attacked by the white pawn) or d5 (not attacked). These are
    // mirrored squares, so the knight's placement & mobility score the same.

    SECTION("Pawn attacking a knight - white to move")
    {
        const auto attacked = from_fen("6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1").value();
        const auto safe     = from_fen("6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) > evaluate(safe, pieceSquareTables));
    }

    SECTION("Pawn attacking a knight - black to move")
    {
        const auto attacked = from_fen("6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1").value();
        const auto safe     = from_fen("6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) < evaluate(safe, pieceSquareTables));
    }
}
