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
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <ranges>
#include <vector>

static constexpr auto TAGS { "[notation][Algebraic]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::game::Position;
using PieceType = chess::pieces::Type;

using chess::notation::from_alg;
using chess::notation::from_fen;
using chess::notation::to_alg;
using chess::notation::to_fen;

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
            REQUIRE(not position.is_checkmate());

            const auto legalMoves = chess::moves::generate(position)
                                  | std::ranges::to<std::vector>();

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
            REQUIRE(not position.is_stalemate());
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
    SECTION("Knights")
    {
        SECTION("Disambig required (by file)")
        {
            const auto position = from_fen("1kr2b1r/ppp1pppp/3q1n2/2np1b2/2B1P3/1NB2N2/PPPPQPPP/R4RK1 w Qk - 0 1");

            { // F knight
                const auto move = from_alg(position, "Nfd4");

                REQUIRE(move.piece == PieceType::Knight);
                REQUIRE(move.to == Square { File::D, Rank::Four });
                REQUIRE(move.from == Square { File::F, Rank::Three });

                REQUIRE(to_alg(position, move) == "Nfd4");
            }
            { // B knight
                const auto move = from_alg(position, "Nbd4");

                REQUIRE(move.piece == PieceType::Knight);
                REQUIRE(move.to == Square { File::D, Rank::Four });
                REQUIRE(move.from == Square { File::B, Rank::Three });

                REQUIRE(to_alg(position, move) == "Nbd4");
            }
        }

        SECTION("Disambig required (by rank)")
        {
            const auto position = from_fen("6r1/2k5/1p1pq1p1/p7/R2QP3/1N3P1P/8/KN6 w - - 0 1");

            { // 3 knight
                const auto move = from_alg(position, "N3d2");

                REQUIRE(move.piece == PieceType::Knight);
                REQUIRE(move.to == Square { File::D, Rank::Two });
                REQUIRE(move.from == Square { File::B, Rank::Three });

                REQUIRE(to_alg(position, move) == "N3d2");
            }
            { // 1 knight
                const auto move = from_alg(position, "N1d2");

                REQUIRE(move.piece == PieceType::Knight);
                REQUIRE(move.to == Square { File::D, Rank::Two });
                REQUIRE(move.from == Square { File::B, Rank::One });

                REQUIRE(to_alg(position, move) == "N1d2");
            }
        }

        SECTION("Disambig not required")
        {
            const auto position = from_fen("rn1qkbnr/ppp1pppp/3p4/8/4P1b1/1NQ2N2/PPPP1PPP/R1BK1B1R w KQkq - 0 1");

            const auto move = from_alg(position, "Nd4");

            REQUIRE(move.piece == PieceType::Knight);
            REQUIRE(move.to == Square { File::D, Rank::Four });
            REQUIRE(move.from == Square { File::B, Rank::Three });

            REQUIRE(to_alg(position, move) == "Nd4");
        }
    }

    SECTION("Rooks")
    {
        SECTION("Disambig required (by file)")
        {
            const auto position = from_fen("r7/8/8/5k2/2R1R3/6n1/1K6/8 w - - 0 1");

            { // E rook
                const auto move = from_alg(position, "Red4");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::D, Rank::Four });
                REQUIRE(move.from == Square { File::E, Rank::Four });

                REQUIRE(to_alg(position, move) == "Red4");
            }
            { // C rook
                const auto move = from_alg(position, "Rcd4");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::D, Rank::Four });
                REQUIRE(move.from == Square { File::C, Rank::Four });

                REQUIRE(to_alg(position, move) == "Rcd4");
            }
            {
                const auto move = from_alg(position, "Ra4");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::A, Rank::Four });
                REQUIRE(move.from == Square { File::C, Rank::Four });

                REQUIRE(to_alg(position, move) == "Ra4");
            }
            {
                const auto move = from_alg(position, "Ra4");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::A, Rank::Four });
                REQUIRE(move.from == Square { File::C, Rank::Four });

                REQUIRE(to_alg(position, move) == "Ra4");
            }
            {
                const auto move = from_alg(position, "Rf4+");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::F, Rank::Four });
                REQUIRE(move.from == Square { File::E, Rank::Four });

                REQUIRE(to_alg(position, move) == "Rf4+");
            }
        }

        SECTION("Disambig required (by rank)")
        {
            const auto position = from_fen("kr6/p7/1r2q3/8/3B4/2Q3N1/3K1P1P/8 b - - 0 1");

            { // 8 rook
                const auto move = from_alg(position, "R8b7");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::B, Rank::Seven });
                REQUIRE(move.from == Square { File::B, Rank::Eight });

                REQUIRE(to_alg(position, move) == "R8b7");
            }
            { // 6 rook
                const auto move = from_alg(position, "R6b7");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::B, Rank::Seven });
                REQUIRE(move.from == Square { File::B, Rank::Six });

                REQUIRE(to_alg(position, move) == "R6b7");
            }
            {
                const auto move = from_alg(position, "Rb3");

                REQUIRE(move.piece == PieceType::Rook);
                REQUIRE(move.to == Square { File::B, Rank::Three });
                REQUIRE(move.from == Square { File::B, Rank::Six });

                REQUIRE(to_alg(position, move) == "Rb3");
            }
        }

        SECTION("Disambig not required")
        {
            const auto position = from_fen("5k2/8/8/q7/6b1/8/1R2R3/3K4 w - - 0 1");

            const auto move = from_alg(position, "Rc2");

            REQUIRE(move.piece == PieceType::Rook);
            REQUIRE(move.to == Square { File::C, Rank::Two });
            REQUIRE(move.from == Square { File::B, Rank::Two });

            REQUIRE(to_alg(position, move) == "Rc2");
        }
    }
}

TEST_CASE("Algebraic notation - pawn pushes", TAGS)
{
    SECTION("Normal")
    {
        const Position startingPosition {};

        const auto move = from_alg(startingPosition, "e3");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::E, Rank::Three });
        REQUIRE(move.from == Square { File::E, Rank::Two });

        REQUIRE(to_alg(startingPosition, move) == "e3");
    }

    SECTION("With check")
    {
        auto position = from_fen("rnbqkb1r/p1p1pppp/3P4/1pp4n/2Q2B2/5N2/PPP1PPPP/RN2KB1R w KQkq - 0 1");

        const auto move = from_alg(position, "d7+");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Seven });
        REQUIRE(move.from == Square { File::D, Rank::Six });

        REQUIRE(to_alg(position, move) == "d7+");

        position.make_move(move);

        REQUIRE(position.is_check());
    }

    SECTION("With checkmate")
    {
        auto position = from_fen("8/2N5/1B6/8/k7/P7/KP6/8 w - - 0 1");

        const auto move = from_alg(position, "b3#");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::B, Rank::Three });
        REQUIRE(move.from == Square { File::B, Rank::Two });

        REQUIRE(to_alg(position, move) == "b3#");

        position.make_move(move);

        REQUIRE(position.is_checkmate());
    }
}

TEST_CASE("Algebraic notation - pawn double pushes", TAGS)
{
    SECTION("Normal")
    {
        Position startingPosition {};

        const auto move = from_alg(startingPosition, "e4");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::E, Rank::Four });
        REQUIRE(move.from == Square { File::E, Rank::Two });

        REQUIRE(to_alg(startingPosition, move) == "e4");

        startingPosition.make_move(move);

        REQUIRE(startingPosition.enPassantTargetSquare.has_value());

        REQUIRE(*startingPosition.enPassantTargetSquare == Square { File::E, Rank::Three });
    }

    SECTION("With check")
    {
        auto position = from_fen("8/8/8/4k3/8/8/2KP4/8 w - - 0 1");

        const auto move = from_alg(position, "d4+");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Four });
        REQUIRE(move.from == Square { File::D, Rank::Two });

        REQUIRE(to_alg(position, move) == "d4+");

        position.make_move(move);

        REQUIRE(position.is_check());

        REQUIRE(position.enPassantTargetSquare.has_value());

        REQUIRE(*position.enPassantTargetSquare == Square { File::D, Rank::Three });
    }

    SECTION("With checkmate")
    {
        auto position = from_fen("4q3/6p1/8/2r5/5k1K/7P/8/6r1 b - - 0 1");

        const auto move = from_alg(position, "g5#");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::G, Rank::Five });
        REQUIRE(move.from == Square { File::G, Rank::Seven });

        REQUIRE(to_alg(position, move) == "g5#");

        position.make_move(move);

        REQUIRE(position.is_checkmate());

        REQUIRE(position.enPassantTargetSquare.has_value());

        REQUIRE(*position.enPassantTargetSquare == Square { File::G, Rank::Six });
    }
}

TEST_CASE("Algebraic notation - pawn captures", TAGS)
{
    SECTION("Normal")
    {
        const auto position = from_fen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");

        const auto move = from_alg(position, "exd5");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Five });
        REQUIRE(move.from == Square { File::E, Rank::Four });

        REQUIRE(to_alg(position, move) == "exd5");
    }

    SECTION("With check")
    {
        auto position = from_fen("r2qkbnr/p2ppppp/npp1P3/1b6/6N1/2Q5/PPPP1PPP/RNB1KB1R w KQkq - 0 1");

        const auto move = from_alg(position, "exd7+");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Seven });
        REQUIRE(move.from == Square { File::E, Rank::Six });

        REQUIRE(to_alg(position, move) == "exd7+");

        position.make_move(move);

        REQUIRE(position.is_check());
    }

    SECTION("With checkmate")
    {
        auto position = from_fen("r1b1kb1r/ppp1p1pp/5p2/6Q1/5q1N/1Pn3p1/P1PPPPPP/R1BnKB1R b KQkq - 0 1");

        const auto move = from_alg(position, "gxf2#");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::F, Rank::Two });
        REQUIRE(move.from == Square { File::G, Rank::Three });

        REQUIRE(to_alg(position, move) == "gxf2#");

        position.make_move(move);

        REQUIRE(position.is_checkmate());
    }
}

TEST_CASE("Algebraic notation - promotion (push)", TAGS)
{
    SECTION("Normal")
    {
        auto position = from_fen("8/1k1P4/8/2r5/8/8/4K3/8 w - - 0 1");

        const auto move = from_alg(position, "d8=Q");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Eight });
        REQUIRE(move.from == Square { File::D, Rank::Seven });

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Queen);

        REQUIRE(to_alg(position, move) == "d8=Q");

        position.make_move(move);

        REQUIRE(not position.whitePieces.pawns.test(Square { File::D, Rank::Seven }));
        REQUIRE(not position.whitePieces.pawns.test(Square { File::D, Rank::Eight }));

        REQUIRE(position.whitePieces.queens.test(Square { File::D, Rank::Eight }));
    }

    SECTION("With check")
    {
        auto position = from_fen("8/1k1P4/8/2r5/8/8/4K3/8 w - - 0 1");

        const auto move = from_alg(position, "d8=N+");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Eight });
        REQUIRE(move.from == Square { File::D, Rank::Seven });

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Knight);

        REQUIRE(to_alg(position, move) == "d8=N+");

        position.make_move(move);

        REQUIRE(not position.whitePieces.pawns.test(Square { File::D, Rank::Seven }));
        REQUIRE(not position.whitePieces.pawns.test(Square { File::D, Rank::Eight }));

        REQUIRE(position.whitePieces.knights.test(Square { File::D, Rank::Eight }));

        REQUIRE(position.is_check());
    }

    SECTION("With checkmate")
    {
        auto position = from_fen("k7/ppP5/8/5K2/8/8/8/8 w - - 0 1");

        const auto move = from_alg(position, "c8=R#");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::C, Rank::Eight });
        REQUIRE(move.from == Square { File::C, Rank::Seven });

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Rook);

        REQUIRE(to_alg(position, move) == "c8=R#");

        position.make_move(move);

        REQUIRE(position.is_checkmate());
    }
}

TEST_CASE("Algebraic notation - promotion (capture)", TAGS)
{
    SECTION("Normal")
    {
        auto position = from_fen("3r4/2K1Pk2/8/8/8/8/8/8 w - - 0 1");

        const auto move = from_alg(position, "exd8=B");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Eight });
        REQUIRE(move.from == Square { File::E, Rank::Seven });

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Bishop);

        REQUIRE(to_alg(position, move) == "exd8=B");

        position.make_move(move);

        REQUIRE(not position.whitePieces.pawns.test(Square { File::E, Rank::Seven }));
        REQUIRE(not position.whitePieces.pawns.test(Square { File::E, Rank::Eight }));
        REQUIRE(not position.whitePieces.pawns.test(Square { File::D, Rank::Eight }));

        REQUIRE(position.whitePieces.bishops.test(Square { File::D, Rank::Eight }));
    }

    SECTION("With check")
    {
        auto position = from_fen("8/8/8/8/8/2k5/4p3/2KQ4 b - - 0 1");

        const auto move = from_alg(position, "exd1=Q+");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::One });
        REQUIRE(move.from == Square { File::E, Rank::Two });

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Queen);

        REQUIRE(to_alg(position, move) == "exd1=Q+");

        position.make_move(move);

        REQUIRE(position.is_check());
    }

    SECTION("With checkmate")
    {
        auto position = from_fen("b2r4/1k1NP3/8/K7/1r6/8/2R5/6B1 w - - 0 1");

        const auto move = from_alg(position, "exd8=N#");

        REQUIRE(move.piece == PieceType::Pawn);
        REQUIRE(move.to == Square { File::D, Rank::Eight });
        REQUIRE(move.from == Square { File::E, Rank::Seven });

        REQUIRE(move.is_promotion());
        REQUIRE(*move.promotedType == PieceType::Knight);

        REQUIRE(to_alg(position, move) == "exd8=N#");

        position.make_move(move);

        REQUIRE(position.is_checkmate());
    }
}

TEST_CASE("Algebraic notation - kingside castling", TAGS)
{
    SECTION("Normal")
    {
        SECTION("White")
        {
            auto position = from_fen("rnbqkb1r/ppp1pppp/3p1n2/8/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1");

            const auto move = from_alg(position, "O-O");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::G, Rank::One });
            REQUIRE(move.from == Square { File::E, Rank::One });

            REQUIRE(to_alg(position, move) == "O-O");

            position.make_move(move);

            REQUIRE(to_fen(position) == "rnbqkb1r/ppp1pppp/3p1n2/8/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 1 1");
        }

        SECTION("Black")
        {
            auto position = from_fen("rnbqk2r/pp1ppppp/2p5/1b3n2/8/1B1P1Q2/PPP1PPPP/RN2KBNR b KQkq - 0 1");

            const auto move = from_alg(position, "O-O");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::G, Rank::Eight });
            REQUIRE(move.from == Square { File::E, Rank::Eight });

            REQUIRE(to_alg(position, move) == "O-O");

            position.make_move(move);

            REQUIRE(to_fen(position) == "rnbq1rk1/pp1ppppp/2p5/1b3n2/8/1B1P1Q2/PPP1PPPP/RN2KBNR w KQ - 1 2");
        }
    }

    SECTION("With check")
    {
        SECTION("White")
        {
            auto position = from_fen("rnbq1knr/ppppp1pp/2b5/8/8/1QNBP3/PPPP2PP/RNB1K2R w KQkq - 0 1");

            const auto move = from_alg(position, "O-O+");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::G, Rank::One });
            REQUIRE(move.from == Square { File::E, Rank::One });

            REQUIRE(to_alg(position, move) == "O-O+");

            position.make_move(move);

            REQUIRE(to_fen(position) == "rnbq1knr/ppppp1pp/2b5/8/8/1QNBP3/PPPP2PP/RNB2RK1 b kq - 1 1");

            REQUIRE(position.is_check());
        }

        SECTION("Black")
        {
            auto position = from_fen("rnbqk2r/ppp1p1pp/7b/3p4/2B1N3/4nK2/PPPPP1PP/RNBQ1R2 b kq - 0 1");

            const auto move = from_alg(position, "O-O+");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::G, Rank::Eight });
            REQUIRE(move.from == Square { File::E, Rank::Eight });

            REQUIRE(to_alg(position, move) == "O-O+");

            position.make_move(move);

            REQUIRE(to_fen(position) == "rnbq1rk1/ppp1p1pp/7b/3p4/2B1N3/4nK2/PPPPP1PP/RNBQ1R2 w - - 1 2");

            REQUIRE(position.is_check());
        }
    }

    SECTION("With checkmate")
    {
        SECTION("White")
        {
            auto position = from_fen("8/8/8/7N/2BQ4/5k2/8/4K2R w K - 0 1");

            const auto move = from_alg(position, "O-O#");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::G, Rank::One });
            REQUIRE(move.from == Square { File::E, Rank::One });

            REQUIRE(to_alg(position, move) == "O-O#");

            position.make_move(move);

            REQUIRE(to_fen(position) == "8/8/8/7N/2BQ4/5k2/8/5RK1 b - - 1 1");

            REQUIRE(position.is_checkmate());
        }

        SECTION("Black")
        {
            auto position = from_fen("4k2r/8/8/8/8/3n2r1/7r/5K2 b k - 0 1");

            const auto move = from_alg(position, "O-O#");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::G, Rank::Eight });
            REQUIRE(move.from == Square { File::E, Rank::Eight });

            REQUIRE(to_alg(position, move) == "O-O#");

            position.make_move(move);

            REQUIRE(to_fen(position) == "5rk1/8/8/8/8/3n2r1/7r/5K2 w - - 1 2");

            REQUIRE(position.is_checkmate());
        }
    }
}

TEST_CASE("Algebraic notation - queenside castling", TAGS)
{
    SECTION("Normal")
    {
        SECTION("White")
        {
            auto position = from_fen("rn1qkbnr/pppp1ppp/3b4/4p3/8/2NP1Q2/PPPBPPPP/R3KBNR w KQkq - 0 1");

            const auto move = from_alg(position, "O-O-O");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::C, Rank::One });
            REQUIRE(move.from == Square { File::E, Rank::One });

            REQUIRE(to_alg(position, move) == "O-O-O");

            position.make_move(move);

            REQUIRE(to_fen(position) == "rn1qkbnr/pppp1ppp/3b4/4p3/8/2NP1Q2/PPPBPPPP/2KR1BNR b kq - 1 1");
        }

        SECTION("Black")
        {
            auto position = from_fen("r3kbnr/ppp1pppp/n7/2qp1b2/8/3PB3/PPPQPPPP/RN2KBNR b KQkq - 0 1");

            const auto move = from_alg(position, "O-O-O");

            REQUIRE(move.is_castling());
            REQUIRE(move.piece == PieceType::King);
            REQUIRE(move.to == Square { File::C, Rank::Eight });
            REQUIRE(move.from == Square { File::E, Rank::Eight });

            REQUIRE(to_alg(position, move) == "O-O-O");

            position.make_move(move);

            REQUIRE(to_fen(position) == "2kr1bnr/ppp1pppp/n7/2qp1b2/8/3PB3/PPPQPPPP/RN2KBNR w KQ - 1 2");
        }
    }

    SECTION("With check")
    {
        auto position = from_fen("rn1k1bnr/ppp1pppp/4b3/5q2/4N3/2B2Q2/PPP1PPPP/R3KBNR w KQkq - 0 1");

        const auto move = from_alg(position, "O-O-O+");

        REQUIRE(move.is_castling());
        REQUIRE(move.piece == PieceType::King);
        REQUIRE(move.to == Square { File::C, Rank::One });
        REQUIRE(move.from == Square { File::E, Rank::One });

        REQUIRE(to_alg(position, move) == "O-O-O+");

        position.make_move(move);

        REQUIRE(to_fen(position) == "rn1k1bnr/ppp1pppp/4b3/5q2/4N3/2B2Q2/PPP1PPPP/2KR1BNR b kq - 1 1");

        REQUIRE(position.is_check());
    }

    SECTION("With checkmate")
    {
        auto position = from_fen("r3kb2/ppp1pppp/8/8/6b1/8/1PP1PPnP/r1NKnBNR b Kq - 0 1");

        const auto move = from_alg(position, "O-O-O#");

        REQUIRE(move.is_castling());
        REQUIRE(move.piece == PieceType::King);
        REQUIRE(move.to == Square { File::C, Rank::Eight });
        REQUIRE(move.from == Square { File::E, Rank::Eight });

        REQUIRE(to_alg(position, move) == "O-O-O#");

        position.make_move(move);

        REQUIRE(to_fen(position) == "2kr1b2/ppp1pppp/8/8/6b1/8/1PP1PPnP/r1NKnBNR w K - 1 2");

        REQUIRE(position.is_checkmate());
    }
}
