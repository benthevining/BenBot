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
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

static constexpr auto TAGS { "[game][Position][threefold]" };

using chess::notation::from_alg;

TEST_CASE("Position - threefold repetitions", TAGS)
{
    chess::game::Position pos {};

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3"));
    pos.make_move(from_alg(pos, "Nc6"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nb1"));
    pos.make_move(from_alg(pos, "Nb8"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3"));
    pos.make_move(from_alg(pos, "Nc6"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nb1"));
    pos.make_move(from_alg(pos, "Nb8"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3"));

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc6"));

    REQUIRE(pos.is_threefold_repetition());
}

TEST_CASE("Position - threefold reps - not threefold if EP possible in starting position", TAGS)
{
    auto pos = chess::notation::from_fen(
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");

    pos.make_move(from_alg(pos, "Be2"));
    pos.make_move(from_alg(pos, "Bd7"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bf1"));
    pos.make_move(from_alg(pos, "Bc8"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Be2"));
    pos.make_move(from_alg(pos, "Bd7"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bf1"));
    pos.make_move(from_alg(pos, "Bc8"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Be2"));

    REQUIRE(! pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bd7"));

    REQUIRE(pos.is_threefold_repetition());
}
