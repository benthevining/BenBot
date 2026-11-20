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

// walks the move tree to the given depth, checking that the incrementally updated
// hash matches a freshly calculated one after every move
void check_incremental_hashes(const chess::game::Position& pos, const size_t depth) // NOLINT(misc-no-recursion)
{
    if (depth == 0uz)
        return;

    for (const auto& move : chess::moves::generate(pos)) {
        const auto newPos = after_move(pos, move);

        REQUIRE(newPos.hash == fresh_hash(newPos));

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
            from_fen("8/2p5/3p4/KP5r/1R3p2/8/4P1P1/8 w - - 0 1").value(),
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
