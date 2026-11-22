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
#include <cstddef>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <random>
#include <string_view>

inline constexpr auto TAGS { "[game][Position][Zobrist]" };

using chess::notation::from_alg;
using chess::notation::from_fen;

TEST_CASE("Zobrist - starting position", TAGS)
{
    const chess::game::Position startPosition { };

    const auto startPos = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
                              .value();

    REQUIRE(startPosition.hash == startPos.hash);
}

TEST_CASE("Zobrist - reaching identical positions", TAGS)
{
    const auto position = from_fen("rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2")
                              .value();

    chess::game::Position pos { };

    pos.make_move(from_alg(pos, "e4").value());
    pos.make_move(from_alg(pos, "d5").value());
    pos.make_move(from_alg(pos, "exd5").value());

    REQUIRE(pos.hash == position.hash);
}

TEST_CASE("Zobrist - hash changes", TAGS)
{
    using chess::moves::generate;

    SECTION("From start pos")
    {
        const chess::game::Position pos { };

        const auto oldHash = pos.hash;

        for (const auto& move : generate(pos)) {
            const auto newPos = after_move(pos, move);

            REQUIRE(newPos.hash != oldHash);
        }
    }

    SECTION("From FEN")
    {
        const auto pos = from_fen(
            "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1")
                             .value();

        const auto oldHash = pos.hash;

        for (const auto& move : generate(pos)) {
            const auto newPos = after_move(pos, move);

            REQUIRE(newPos.hash != oldHash);
        }
    }
}

TEST_CASE("Zobrist - loading identical FENs", TAGS)
{
    static constexpr std::string_view fen {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQq - 0 1"
    };

    const auto pos1 = from_fen(fen).value();
    const auto pos2 = from_fen(fen).value();

    REQUIRE(pos1.hash == pos2.hash);

    const auto pos3 = from_fen(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQq - 0 1")
                          .value();

    REQUIRE(pos3.hash != pos1.hash);
}

TEST_CASE("Zobrist - repeated positions", TAGS)
{
    chess::game::Position pos { };

    const auto origHash = pos.hash;

    pos.make_move(from_alg(pos, "Nf3").value());
    pos.make_move(from_alg(pos, "Nf6").value());
    pos.make_move(from_alg(pos, "Ng1").value());
    pos.make_move(from_alg(pos, "Ng8").value());

    REQUIRE(pos.hash == origHash);
}

TEST_CASE("Zobrist - repeated positions, but original had EP possibility", TAGS)
{
    auto pos = from_fen("rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1")
                   .value();

    const auto origHash = pos.hash;

    pos.make_move(from_alg(pos, "Bd7").value());
    pos.make_move(from_alg(pos, "Be2").value());
    pos.make_move(from_alg(pos, "Bc8").value());
    pos.make_move(from_alg(pos, "Bf1").value());

    REQUIRE(pos.hash != origHash);
}

namespace {

[[nodiscard]] auto fresh_hash(chess::game::Position pos) -> chess::game::Position::Hash
{
    pos.refresh_zobrist();
    return pos.hash;
}

[[nodiscard]] auto fresh_pawn_hash(chess::game::Position pos) -> chess::game::Position::Hash
{
    pos.refresh_zobrist();
    return pos.pawnHash;
}

// walks the move tree to the given depth, checking that the incrementally updated
// hashes match freshly calculated ones after every move
void check_incremental_hashes(const chess::game::Position& pos, const size_t depth) // NOLINT(misc-no-recursion)
{
    if (depth == 0uz)
        return;

    for (const auto& move : chess::moves::generate(pos)) {
        const auto newPos = after_move(pos, move);

        REQUIRE(newPos.hash == fresh_hash(newPos));
        REQUIRE(newPos.pawnHash == fresh_pawn_hash(newPos));

        check_incremental_hashes(newPos, depth - 1uz);
    }
}

} // namespace

TEST_CASE("Zobrist - castling", TAGS)
{
    using chess::pieces::Color;

    SECTION("White")
    {
        const auto pos = from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1").value();

        SECTION("Kingside")
        {
            const auto newPos = after_move(pos, chess::moves::castle_kingside(Color::White));

            REQUIRE(newPos.hash == fresh_hash(newPos));
        }

        SECTION("Queenside")
        {
            const auto newPos = after_move(pos, chess::moves::castle_queenside(Color::White));

            REQUIRE(newPos.hash == fresh_hash(newPos));
        }
    }

    SECTION("Black")
    {
        const auto pos = from_fen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1").value();

        SECTION("Kingside")
        {
            const auto newPos = after_move(pos, chess::moves::castle_kingside(Color::Black));

            REQUIRE(newPos.hash == fresh_hash(newPos));
        }

        SECTION("Queenside")
        {
            const auto newPos = after_move(pos, chess::moves::castle_queenside(Color::Black));

            REQUIRE(newPos.hash == fresh_hash(newPos));
        }
    }
}

TEST_CASE("Zobrist - transposition through castling", TAGS)
{
    chess::game::Position pos { };

    for (const auto* move : { "e4", "e5", "Nf3", "Nc6", "Bc4", "Bc5", "O-O", "Nf6" })
        pos.make_move(from_alg(pos, move).value());

    const auto position = from_fen("r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 w kq - 6 5")
                              .value();

    REQUIRE(pos.hash == position.hash);
}

TEST_CASE("Zobrist - incremental updates match fresh calculation", TAGS)
{
    static constexpr auto DEPTH = 3uz;

    SECTION("Starting position")
    {
        check_incremental_hashes(chess::game::Position { }, DEPTH);
    }

    SECTION("Kiwipete")
    {
        check_incremental_hashes(
            from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value(),
            DEPTH);
    }

    SECTION("En passant")
    {
        check_incremental_hashes(
            from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1").value(),
            DEPTH + 1uz);
    }

    SECTION("Promotions")
    {
        check_incremental_hashes(
            from_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1").value(),
            DEPTH);
    }

    SECTION("Promotions & castling")
    {
        check_incremental_hashes(
            from_fen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8").value(),
            DEPTH);
    }
}

TEST_CASE("Zobrist - pawn hash incremental updates match fresh calculation", TAGS)
{
    SECTION("Pawn moves, captures, en passant & promotions")
    {
        // each of these positions has a legal move of the named type for the side to move
        for (const auto* fen : {
                 "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",       // pawn push & double push
                 "4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1",     // capture by a pawn
                 "4k3/8/8/3p4/8/8/8/3RK3 w - - 0 1",      // capture of a pawn by a piece
                 "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1",     // en passant
                 "4k3/2P5/8/8/8/8/8/4K3 w - - 0 1",       // promotion
                 "1r2k3/2P5/8/8/8/8/8/4K3 w - - 0 1",     // capture-promotion
                 "4k3/8/8/8/8/8/2p5/1R2K3 b - - 0 1",     // Black capture-promotion
                 "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1" }) { // Black en passant
            check_incremental_hashes(from_fen(fen).value(), 3uz);
        }
    }

    SECTION("Random playouts")
    {
        std::mt19937 rng { 692u }; // NOLINT(cert-msc32-c,cert-msc51-cpp) - deterministic on purpose

        static constexpr auto NUM_GAMES = 200uz;
        static constexpr auto MAX_PLIES = 300uz;

        for (auto game = 0uz; game < NUM_GAMES; ++game) {
            chess::game::Position pos { };

            for (auto ply = 0uz; ply < MAX_PLIES; ++ply) {
                const auto moves = chess::moves::generate(pos);

                if (moves.empty())
                    break;

                std::uniform_int_distribution<size_t> dist { 0uz, moves.size() - 1uz };

                pos.make_move(moves[dist(rng)]);

                REQUIRE(pos.hash == fresh_hash(pos));
                REQUIRE(pos.pawnHash == fresh_pawn_hash(pos));
            }
        }
    }
}

TEST_CASE("Zobrist - pawn hash only depends on the pawns", TAGS)
{
    SECTION("Same pawns, different pieces")
    {
        const auto pos1 = from_fen("r3k2r/pp3ppp/2n5/3p4/3P4/2N5/PP3PPP/R3K2R w KQkq - 0 1").value();
        const auto pos2 = from_fen("4k3/pp3ppp/8/3p4/3P4/8/PP3PPP/4K3 b - - 3 20").value();
        const auto pos3 = from_fen("1q2k1b1/pp3ppp/8/3p4/3P4/5N2/PP3PPP/2Q1K3 w - - 0 1").value();

        REQUIRE(pos1.hash != pos2.hash);
        REQUIRE(pos1.pawnHash == pos2.pawnHash);
        REQUIRE(pos1.pawnHash == pos3.pawnHash);
    }

    SECTION("Moving pieces doesn't change it")
    {
        chess::game::Position pos { };

        const auto origPawnHash = pos.pawnHash;

        for (const auto* move : { "Nf3", "Nc6", "Ng1", "Nb8", "Nc3" })
            pos.make_move(from_alg(pos, move).value());

        REQUIRE(pos.pawnHash == origPawnHash);

        pos.make_move(from_alg(pos, "e5").value());

        REQUIRE(pos.pawnHash != origPawnHash);
    }

    SECTION("Null moves don't change it")
    {
        const auto pos = from_fen("rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1").value();

        const auto afterNull = after_null_move(pos);

        REQUIRE(afterNull.hash != pos.hash);
        REQUIRE(afterNull.pawnHash == pos.pawnHash);
    }

    SECTION("Different pawns give a different hash")
    {
        const auto pos1 = from_fen("4k3/pp3ppp/8/3p4/3P4/8/PP3PPP/4K3 w - - 0 1").value();
        const auto pos2 = from_fen("4k3/pp3ppp/8/3p4/4P3/8/PP3PPP/4K3 w - - 0 1").value();
        const auto pos3 = from_fen("4k3/pp3ppp/8/3P4/3p4/8/PP3PPP/4K3 w - - 0 1").value();

        REQUIRE(pos1.pawnHash != pos2.pawnHash);
        REQUIRE(pos1.pawnHash != pos3.pawnHash); // same squares, colours swapped
    }

    SECTION("No pawns")
    {
        const auto pos1 = from_fen("4k3/8/8/8/8/8/8/4K3 w - - 0 1").value();
        const auto pos2 = from_fen("r3k3/8/8/8/8/8/8/4K2R b - - 0 1").value();

        REQUIRE(pos1.pawnHash != 0uz);
        REQUIRE(pos1.pawnHash == pos2.pawnHash);
    }
}

TEST_CASE("Zobrist - pawn hash of flipped positions", TAGS)
{
    const auto pos      = from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();
    const auto expected = from_fen("r3k2r/pppbbppp/2n2q1P/1P2p3/3pn3/BN2PNP1/P1PPQPB1/R3K2R b KQkq - 0 1").value();

    const auto flippedPos = flipped(pos);

    REQUIRE(flippedPos.pawnHash == fresh_pawn_hash(flippedPos));
    REQUIRE(flippedPos.pawnHash == expected.pawnHash);
    REQUIRE(flippedPos.pawnHash != pos.pawnHash);
    REQUIRE(flipped(flippedPos).pawnHash == pos.pawnHash);
}
