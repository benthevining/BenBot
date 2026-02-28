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
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/ICCF.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <print>

inline constexpr auto TAGS { "[notation][ICCF]" };

using PieceType = chess::pieces::Type;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::game::Position;
using chess::notation::from_fen;
using chess::notation::from_iccf;
using chess::notation::to_iccf;

TEST_CASE("ICCF notation", TAGS)
{
    const Position startingPosition { };

    SECTION("Pawn push: e4")
    {
        const auto move = from_iccf(startingPosition, "5254").value();

        REQUIRE(move.piece() == PieceType::Pawn);
        REQUIRE(move.to() == Square { File::E, Rank::Four });
        REQUIRE(move.from() == Square { File::E, Rank::Two });

        REQUIRE(to_iccf(move) == "5254");
    }

    SECTION("Promotion to rook")
    {
        const auto pos = from_fen("8/5P2/2k5/8/8/8/1K6/8 w - - 0 1").value();

        const auto move = from_iccf(pos, "67682").value();

        REQUIRE(move.piece() == PieceType::Pawn);
        REQUIRE(move.to() == Square { File::F, Rank::Eight });
        REQUIRE(move.from() == Square { File::F, Rank::Seven });

        REQUIRE(move.is_promotion());
        REQUIRE(move.promoted_type().value() == PieceType::Rook);

        REQUIRE(to_iccf(move) == "67682");
    }
}

TEST_CASE("ICCF notation: castling", TAGS)
{
    const auto pos = from_fen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1").value();

    SECTION("Kingside")
    {
        SECTION("White")
        {
            const auto move = from_iccf(pos, "5171").value();

            REQUIRE(move.piece() == PieceType::King);
            REQUIRE(move.to() == Square { File::G, Rank::One });
            REQUIRE(move.from() == Square { File::E, Rank::One });

            REQUIRE(move.is_castling());

            REQUIRE(to_iccf(move) == "5171");
        }

        SECTION("Black")
        {
            const auto move = from_iccf(
                after_null_move(pos), "5878")
                                  .value();

            REQUIRE(move.piece() == PieceType::King);
            REQUIRE(move.to() == Square { File::G, Rank::Eight });
            REQUIRE(move.from() == Square { File::E, Rank::Eight });

            REQUIRE(move.is_castling());

            REQUIRE(to_iccf(move) == "5878");
        }
    }

    SECTION("Queenside")
    {
        SECTION("White")
        {
            const auto move = from_iccf(pos, "5131").value();

            REQUIRE(move.piece() == PieceType::King);
            REQUIRE(move.to() == Square { File::C, Rank::One });
            REQUIRE(move.from() == Square { File::E, Rank::One });

            REQUIRE(move.is_castling());

            REQUIRE(to_iccf(move) == "5131");
        }

        SECTION("Black")
        {
            const auto move = from_iccf(
                after_null_move(pos), "5838")
                                  .value();

            REQUIRE(move.piece() == PieceType::King);
            REQUIRE(move.to() == Square { File::C, Rank::Eight });
            REQUIRE(move.from() == Square { File::E, Rank::Eight });

            REQUIRE(move.is_castling());

            REQUIRE(to_iccf(move) == "5838");
        }
    }
}
