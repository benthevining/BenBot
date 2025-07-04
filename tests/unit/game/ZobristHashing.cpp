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
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <string_view>

static constexpr auto TAGS { "[game][Position][Zobrist]" };

using chess::notation::from_alg;
using chess::notation::from_fen;

TEST_CASE("Zobrist - starting position", TAGS)
{
    const chess::game::Position startPosition {};

    const auto startPos = from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    REQUIRE(startPosition.hash == startPos.hash);
}

TEST_CASE("Zobrist - reaching identical positions", TAGS)
{
    const auto position = from_fen("rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");

    chess::game::Position pos {};

    pos.make_move(from_alg(pos, "e4"));
    pos.make_move(from_alg(pos, "d5"));
    pos.make_move(from_alg(pos, "exd5"));

    REQUIRE(pos.hash == position.hash);
}

TEST_CASE("Zobrist - hash changes", TAGS)
{
    using chess::moves::generate;

    SECTION("From start pos")
    {
        const chess::game::Position pos {};

        const auto oldHash = pos.hash;

        for (const auto& move : generate(pos)) {
            const auto newPos = chess::game::after_move(pos, move);

            REQUIRE(newPos.hash != oldHash);
        }
    }

    SECTION("From FEN")
    {
        const auto pos = from_fen(
            "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");

        const auto oldHash = pos.hash;

        for (const auto& move : generate(pos)) {
            const auto newPos = chess::game::after_move(pos, move);

            REQUIRE(newPos.hash != oldHash);
        }
    }
}

TEST_CASE("Zobrist - loading identical FENs", TAGS)
{
    static constexpr std::string_view fen {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQq - 0 1"
    };

    const auto pos1 = from_fen(fen);
    const auto pos2 = from_fen(fen);

    REQUIRE(pos1.hash == pos2.hash);

    const auto pos3 = from_fen(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQq - 0 1");

    REQUIRE(pos3.hash != pos1.hash);
}

TEST_CASE("Zobrist - repeated positions", TAGS)
{
    chess::game::Position pos {};

    const auto origHash = pos.hash;

    pos.make_move(from_alg(pos, "Nf3"));
    pos.make_move(from_alg(pos, "Nf6"));
    pos.make_move(from_alg(pos, "Ng1"));
    pos.make_move(from_alg(pos, "Ng8"));

    REQUIRE(pos.hash == origHash);
}

TEST_CASE("Zobrist - repeated positions, but original had EP possibility", TAGS)
{
    auto pos = from_fen("rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

    const auto origHash = pos.hash;

    pos.make_move(from_alg(pos, "Bd7"));
    pos.make_move(from_alg(pos, "Be2"));
    pos.make_move(from_alg(pos, "Bc8"));
    pos.make_move(from_alg(pos, "Bf1"));

    REQUIRE(pos.hash != origHash);
}
