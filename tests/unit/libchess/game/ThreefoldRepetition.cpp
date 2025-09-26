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

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3").value());
    pos.make_move(from_alg(pos, "Nc6").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nb1").value());
    pos.make_move(from_alg(pos, "Nb8").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3").value());
    pos.make_move(from_alg(pos, "Nc6").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nb1").value());
    pos.make_move(from_alg(pos, "Nb8").value());

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3").value());

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc6").value());

    REQUIRE(pos.is_threefold_repetition());
}

TEST_CASE("Threefold repetition from differing moves", TAGS)
{
    chess::game::Position pos {};

    pos.make_move(from_alg(pos, "Nf3").value());
    pos.make_move(from_alg(pos, "Nf6").value());
    pos.make_move(from_alg(pos, "Ng1").value());
    pos.make_move(from_alg(pos, "Ng8").value());
    pos.make_move(from_alg(pos, "Nc3").value());
    pos.make_move(from_alg(pos, "Nc6").value());
    pos.make_move(from_alg(pos, "Nb1").value());
    pos.make_move(from_alg(pos, "Nb8").value());

    REQUIRE(pos.is_threefold_repetition());
}

TEST_CASE("Position - threefold reps - not threefold if EP possible in starting position", TAGS)
{
    auto pos = chess::notation::from_fen(
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1");

    pos.make_move(from_alg(pos, "Be2").value());
    pos.make_move(from_alg(pos, "Bd7").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bf1").value());
    pos.make_move(from_alg(pos, "Bc8").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Be2").value());
    pos.make_move(from_alg(pos, "Bd7").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bf1").value());
    pos.make_move(from_alg(pos, "Bc8").value());

    REQUIRE(not pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Be2").value());

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bd7").value());

    REQUIRE(pos.is_threefold_repetition());
}
