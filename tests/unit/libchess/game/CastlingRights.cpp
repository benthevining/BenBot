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
#include <libchess/game/CastlingRights.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

static constexpr auto TAGS { "[game][CastlingRights]" };

using chess::game::CastlingRights;

TEST_CASE("Castling rights - default", TAGS)
{
    static constexpr CastlingRights rights;

    STATIC_REQUIRE(rights.either());
    STATIC_REQUIRE(rights.kingside);
    STATIC_REQUIRE(rights.queenside);
}

TEST_CASE("Castling rights - king_moved()", TAGS)
{
    CastlingRights rights;

    REQUIRE(rights.either());

    rights.king_moved();

    REQUIRE(not rights.either());
}

TEST_CASE("Castling rights - rook_moved()", TAGS)
{
    CastlingRights rights;

    REQUIRE(rights.either());

    SECTION("Kingside")
    {
        rights.rook_moved(true);

        REQUIRE(not rights.kingside);
        REQUIRE(rights.either());
    }

    SECTION("Queenside")
    {
        rights.rook_moved(false);

        REQUIRE(not rights.queenside);
        REQUIRE(rights.either());
    }
}

TEST_CASE("Castling rights - lost when rook captured", TAGS)
{
    using chess::notation::from_alg;
    using chess::notation::from_fen;

    SECTION("White - kingside")
    {
        auto position = from_fen("r1b1kb1r/ppp2ppp/2q1p3/2np4/5P2/1NP1N1P1/PP1PPnBP/R1BQK2R b KQkq - 0 1");

        REQUIRE(position.whiteCastlingRights.kingside);

        const auto move = from_alg(position, "Nxh1");

        position.make_move(move);

        REQUIRE(not position.whiteCastlingRights.kingside);
    }

    SECTION("White - queenside")
    {
        auto position = from_fen("r2qk2r/ppp2ppp/2np1n2/4p3/3PPNb1/1Q6/PbPB1PPP/R3KBNR b KQkq - 0 1");

        REQUIRE(position.whiteCastlingRights.queenside);

        const auto move = from_alg(position, "Bxa1");

        position.make_move(move);

        REQUIRE(not position.whiteCastlingRights.queenside);
    }

    SECTION("Black - kingside")
    {
        auto position = from_fen("rnbqk2r/ppp2pBp/4p3/2bp1n2/3P4/8/PPP1PPPP/RNBQK1NR w KQkq - 0 1");

        REQUIRE(position.blackCastlingRights.kingside);

        const auto move = from_alg(position, "Bxh8");

        position.make_move(move);

        REQUIRE(not position.blackCastlingRights.kingside);
    }

    SECTION("Black - queenside")
    {
        auto position = from_fen("r3kbnr/pQ3ppp/2nq4/3pp3/5Bb1/1N1P4/PPP1PPPP/R3KBNR w KQkq - 0 1");

        REQUIRE(position.blackCastlingRights.queenside);

        const auto move = from_alg(position, "Qxa8");

        position.make_move(move);

        REQUIRE(not position.blackCastlingRights.queenside);
    }
}
