/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/PieceTypes.hpp>

static constexpr auto TAGS { "[notation][UCI]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

using PieceType = chess::pieces::Type;

using chess::notation::from_fen;
using chess::notation::from_uci;
using chess::notation::to_uci;

TEST_CASE("UCI notation - normal moves", TAGS)
{
    static constexpr chess::game::Position startingPosition {};

    SECTION("Pawn move")
    {
        const auto move = from_uci(startingPosition, "e2e4");

        REQUIRE(move.from == Square { File::E, Rank::Two });
        REQUIRE(move.to == Square { File::E, Rank::Four });
        REQUIRE(move.piece == PieceType::Pawn);

        REQUIRE(to_uci(move) == "e2e4");
    }

    SECTION("Piece move")
    {
        const auto move = from_uci(startingPosition, "g1f3");

        REQUIRE(move.from == Square { File::G, Rank::One });
        REQUIRE(move.to == Square { File::F, Rank::Three });
        REQUIRE(move.piece == PieceType::Knight);

        REQUIRE(to_uci(move) == "g1f3");
    }
}

TEST_CASE("UCI notation - captures", TAGS)
{
    SECTION("Pawn capture")
    {
        const auto position = from_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");

        const auto move = from_uci(position, "e4d5");

        REQUIRE(move.from == Square { File::E, Rank::Four });
        REQUIRE(move.to == Square { File::D, Rank::Five });
        REQUIRE(move.piece == PieceType::Pawn);

        REQUIRE(to_uci(move) == "e4d5");
    }

    SECTION("Piece capture")
    {
        const auto position = from_fen("rn2kbnr/ppp1pppp/3q4/3p4/4P1b1/2N2N2/PPPPQPPP/R1B1KB1R b KQkq - 5 4");

        const auto move = from_uci(position, "g4f3");

        REQUIRE(move.from == Square { File::G, Rank::Four });
        REQUIRE(move.to == Square { File::F, Rank::Three });
        REQUIRE(move.piece == PieceType::Bishop);

        REQUIRE(to_uci(move) == "g4f3");
    }
}

TEST_CASE("UCI notation - check", TAGS)
{
    auto position = from_fen("3rkbnr/ppp1ppp1/2nq4/3p2Np/4P1b1/2N2Q2/PPPP1PPP/R1B1KB1R w KQk - 0 7");

    const auto move = from_uci(position, "f3f7");

    REQUIRE(move.from == Square { File::F, Rank::Three });
    REQUIRE(move.to == Square { File::F, Rank::Seven });
    REQUIRE(move.piece == PieceType::Queen);

    REQUIRE(to_uci(move) == "f3f7");

    position.make_move(move);

    REQUIRE(position.is_check());
    REQUIRE(! position.is_checkmate());
}

TEST_CASE("UCI notation - checkmate", TAGS)
{
    auto position = from_fen("3rkbnr/pppqppp1/2n5/1N1p2Np/4P1b1/5Q2/PPPP1PPP/R1B1KB1R w KQk - 2 8");

    const auto move = from_uci(position, "f3f7");

    REQUIRE(move.from == Square { File::F, Rank::Three });
    REQUIRE(move.to == Square { File::F, Rank::Seven });
    REQUIRE(move.piece == PieceType::Queen);

    REQUIRE(to_uci(move) == "f3f7");

    position.make_move(move);

    REQUIRE(position.is_check());
    REQUIRE(position.is_checkmate());
}

TEST_CASE("UCI notation - castle kingside", TAGS)
{
    SECTION("White")
    {
        const auto position = from_fen("r1bqkbnr/pppp1ppp/2n5/4p3/8/2BP1N2/PPP1PPPP/RNBQK2R w KQkq - 0 1");

        const auto move = from_uci(position, "e1g1");

        REQUIRE(move.from == Square { File::E, Rank::One });
        REQUIRE(move.to == Square { File::G, Rank::One });
        REQUIRE(move.piece == PieceType::King);

        REQUIRE(to_uci(move) == "e1g1");

        REQUIRE(move.is_castling());
    }

    SECTION("Black")
    {
        const auto position = from_fen("rnbqk2r/ppp2ppp/2bp1n2/4p3/2BP4/4PN2/PPP2PPP/RNBQ1RK1 b Qkq - 0 1");

        const auto move = from_uci(position, "e8g8");

        REQUIRE(move.from == Square { File::E, Rank::Eight });
        REQUIRE(move.to == Square { File::G, Rank::Eight });
        REQUIRE(move.piece == PieceType::King);

        REQUIRE(to_uci(move) == "e8g8");

        REQUIRE(move.is_castling());
    }
}

TEST_CASE("UCI notation - castle queenside", TAGS)
{
    SECTION("White")
    {
        const auto position = from_fen("rnb1kb1r/pp1pp1pp/1qp1np2/8/3P1B2/2N5/PPPQPPPP/R3KBNR w KQkq - 0 1");

        const auto move = from_uci(position, "e1c1");

        REQUIRE(move.from == Square { File::E, Rank::One });
        REQUIRE(move.to == Square { File::C, Rank::One });
        REQUIRE(move.piece == PieceType::King);

        REQUIRE(to_uci(move) == "e1c1");

        REQUIRE(move.is_castling());
    }

    SECTION("Black")
    {
        const auto position = from_fen("r3kbnr/pppqpppp/2np4/8/3P1Bb1/2N1P3/PPP2PPP/R2QKBNR b KQkq - 0 1");

        const auto move = from_uci(position, "e8c8");

        REQUIRE(move.from == Square { File::E, Rank::Eight });
        REQUIRE(move.to == Square { File::C, Rank::Eight });
        REQUIRE(move.piece == PieceType::King);

        REQUIRE(to_uci(move) == "e8c8");

        REQUIRE(move.is_castling());
    }
}

TEST_CASE("UCI notation - promotions", TAGS)
{
    SECTION("Push")
    {
        const auto position = from_fen("8/8/2rk4/8/6Q1/1K2N3/2p5/8 b - - 0 1");

        const auto move = from_uci(position, "c2c1b");

        REQUIRE(move.from == Square { File::C, Rank::Two });
        REQUIRE(move.to == Square { File::C, Rank::One });
        REQUIRE(move.piece == PieceType::Pawn);

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Bishop);

        REQUIRE(to_uci(move) == "c2c1b");
    }

    SECTION("Capture")
    {
        const auto position = from_fen("4r3/1k1K1P2/8/2qN4/8/8/8/8 w - - 0 1");

        const auto move = from_uci(position, "f7g8q");

        REQUIRE(move.from == Square { File::F, Rank::Seven });
        REQUIRE(move.to == Square { File::G, Rank::Eight });
        REQUIRE(move.piece == PieceType::Pawn);

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Queen);

        REQUIRE(to_uci(move) == "f7g8q");
    }
}
