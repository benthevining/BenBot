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
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/PieceTypes.hpp>

static constexpr auto TAGS { "[notation][Algebraic]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::game::Position;
using PieceType = chess::pieces::Type;

using chess::notation::from_alg;
using chess::notation::from_fen;
using chess::notation::to_alg;

// castle kingside (with check, mate)
// castle queenside (with check, mate)
// push promotion (with check, mate)
// capture promotion (with check, mate)
// pawn push (with check, mate)
// pawn double push (with check, mate)
// pawn capture (with check, mate)

TEST_CASE("Algebraic notation - piece moves", TAGS)
{
    SECTION("Knights")
    {
        Position position {};

        {
            const auto move = from_alg(position, "Nc3");

            REQUIRE(move.piece == PieceType::Knight);
            REQUIRE(move.to == Square { File::C, Rank::Three });
            REQUIRE(move.from == Square { File::B, Rank::One });

            REQUIRE(to_alg(position, move) == "Nc3");

            position.make_move(move);
        }

        const auto move = from_alg(position, "Nf6");

        REQUIRE(move.piece == PieceType::Knight);
        REQUIRE(move.to == Square { File::F, Rank::Six });
        REQUIRE(move.from == Square { File::G, Rank::Eight });

        REQUIRE(to_alg(position, move) == "Nf6");
    }

    SECTION("Bishops")
    {
        auto position = from_fen("8/4b2P/r3k3/4qn2/1Q2P3/3pRK2/3B4/8 w - - 0 1");

        {
            const auto move = from_alg(position, "Bc3");

            REQUIRE(move.piece == PieceType::Bishop);
            REQUIRE(move.to == Square { File::C, Rank::Three });
            REQUIRE(move.from == Square { File::D, Rank::Two });

            REQUIRE(to_alg(position, move) == "Bc3");

            position.make_move(move);
        }

        {
            const auto move = from_alg(position, "Bxb4");

            REQUIRE(move.piece == PieceType::Bishop);
            REQUIRE(move.to == Square { File::B, Rank::Four });
            REQUIRE(move.from == Square { File::E, Rank::Seven });

            REQUIRE(to_alg(position, move) == "Bxb4");

            REQUIRE(position.is_capture(move));

            position.make_move(move);
        }

        {
            const auto move = from_alg(position, "Bxe5");

            REQUIRE(move.piece == PieceType::Bishop);
            REQUIRE(move.to == Square { File::E, Rank::Five });
            REQUIRE(move.from == Square { File::C, Rank::Three });

            REQUIRE(to_alg(position, move) == "Bxe5");

            REQUIRE(position.is_capture(move));

            position.make_move(move);

            REQUIRE(position.blackPieces.queens.none());
            REQUIRE(position.whitePieces.queens.none());
        }
    }

    SECTION("Rooks")
    {
        auto position = from_fen("r7/8/3n1r2/4k3/2bRppqP/1Pr5/3KNB2/R7 w - - 0 1");

        {
            const auto move = from_alg(position, "Rg1");

            REQUIRE(move.piece == PieceType::Rook);
            REQUIRE(move.to == Square { File::G, Rank::One });
            REQUIRE(move.from == Square { File::A, Rank::One });

            REQUIRE(to_alg(position, move) == "Rg1");

            position.make_move(move);
        }

        {
            const auto move = from_alg(position, "Ra2+");

            REQUIRE(move.piece == PieceType::Rook);
            REQUIRE(move.to == Square { File::A, Rank::Two });
            REQUIRE(move.from == Square { File::A, Rank::Eight });

            REQUIRE(to_alg(position, move) == "Ra2+");

            position.make_move(move);

            REQUIRE(position.is_check());
        }
    }

    SECTION("Queens")
    {
        SECTION("Check")
        {
            auto position = from_fen("r2qkbnr/p1p1pppp/2np4/1p6/2B1P1b1/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1");

            {
                const auto move = from_alg(position, "Qxf7+");

                REQUIRE(move.piece == PieceType::Queen);
                REQUIRE(move.to == Square { File::F, Rank::Seven });
                REQUIRE(move.from == Square { File::F, Rank::Three });

                REQUIRE(to_alg(position, move) == "Qxf7+");

                position.make_move(move);
            }

            REQUIRE(position.is_check());
            REQUIRE(! position.is_checkmate());

            const auto legalMoves = chess::moves::generate(position);

            REQUIRE(legalMoves.size() == 1uz);

            const auto move = legalMoves.front();

            REQUIRE(to_alg(position, move) == "Kd7");
        }

        SECTION("Checkmate")
        {
            auto position = from_fen("r1bqk1nr/pppnpp1p/3p2pb/8/8/1B3Q2/PPPPPPPP/RNB1K1NR w KQkq - 0 1");

            const auto move = from_alg(position, "Qxf7#");

            REQUIRE(move.piece == PieceType::Queen);
            REQUIRE(move.to == Square { File::F, Rank::Seven });
            REQUIRE(move.from == Square { File::F, Rank::Three });

            REQUIRE(to_alg(position, move) == "Qxf7#");

            position.make_move(move);

            REQUIRE(position.is_check());
            REQUIRE(position.is_checkmate());
            REQUIRE(! position.is_stalemate());
        }
    }

    SECTION("King")
    {
        auto position = from_fen("8/3k1p2/2P2rp1/4K3/R7/6n1/8/8 b - - 0 1");

        REQUIRE(position.is_check());

        {
            const auto move = from_alg(position, "Kxc6");

            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::C, Rank::Six });
            REQUIRE(move.from == Square { File::D, Rank::Seven });

            REQUIRE(to_alg(position, move) == "Kxc6");

            position.make_move(move);
        }

        REQUIRE(! position.is_check());

        {
            const auto move = from_alg(position, "Kxf6");

            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::F, Rank::Six });
            REQUIRE(move.from == Square { File::E, Rank::Five });

            REQUIRE(to_alg(position, move) == "Kxf6");
        }
    }
}

TEST_CASE("Algebraic notation - piece moves with disambiguation", TAGS)
{
}
