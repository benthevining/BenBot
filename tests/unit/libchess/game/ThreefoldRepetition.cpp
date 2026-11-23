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
#include <libchess/game/ThreefoldChecker.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

inline constexpr auto TAGS { "[game][Position][threefold]" };

using chess::notation::from_alg;

TEST_CASE("Position - threefold repetitions", TAGS)
{
    chess::game::Position pos { };

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3").value());
    pos.make_move(from_alg(pos, "Nc6").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nb1").value());
    pos.make_move(from_alg(pos, "Nb8").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3").value());
    pos.make_move(from_alg(pos, "Nc6").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nb1").value());
    pos.make_move(from_alg(pos, "Nb8").value());

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc3").value());

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Nc6").value());

    REQUIRE(pos.is_threefold_repetition());
}

TEST_CASE("Threefold repetition whose first occurrence was more than 50 plies ago", TAGS)
{
    // the fifty-move rule allows up to 100 plies without a capture or pawn move, so a
    // repetition can span that many plies
    chess::game::ThreefoldChecker checker;

    static constexpr auto REPEATED { 1000uz };

    checker.reset(REPEATED);

    for (auto hash = 1uz; hash <= 90uz; ++hash)
        checker.push(hash);

    checker.push(REPEATED);

    REQUIRE_FALSE(checker.is_threefold());
    REQUIRE(checker.repetition_count() == 2uz);

    checker.push(91uz);
    checker.push(92uz);
    checker.push(93uz);
    checker.push(REPEATED);

    REQUIRE(checker.is_threefold());
    REQUIRE(checker.repetition_count() == 3uz);
}

TEST_CASE("Threefold repetition from differing moves", TAGS)
{
    chess::game::Position pos { };

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
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1")
                   .value();

    pos.make_move(from_alg(pos, "Be2").value());
    pos.make_move(from_alg(pos, "Bd7").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bf1").value());
    pos.make_move(from_alg(pos, "Bc8").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Be2").value());
    pos.make_move(from_alg(pos, "Bd7").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bf1").value());
    pos.make_move(from_alg(pos, "Bc8").value());

    REQUIRE_FALSE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Be2").value());

    REQUIRE(pos.is_threefold_repetition());

    pos.make_move(from_alg(pos, "Bd7").value());

    REQUIRE(pos.is_threefold_repetition());
}
