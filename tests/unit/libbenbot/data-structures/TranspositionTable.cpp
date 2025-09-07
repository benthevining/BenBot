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

// probe_eval()

#include <catch2/catch_test_macros.hpp>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

static constexpr auto TAGS { "[data-structures][TranspositionTable]" };

using ben_bot::TranspositionTable;
using EvalType = TranspositionTable::Record::EvalType;

namespace notation = chess::notation;

TEST_CASE("Transposition table - find()", TAGS)
{
    static const chess::game::Position startPos {};

    const auto pos2 = notation::from_fen("8/8/4n3/2B1k1p1/3Pn3/2K5/5R2/8 b - - 0 1");

    TranspositionTable table;

    REQUIRE(table.find(startPos) == nullptr);
    REQUIRE(table.find(pos2) == nullptr);

    table.store(startPos, {});

    REQUIRE(table.find(startPos) != nullptr);
    REQUIRE(table.find(pos2) == nullptr);

    table.store(pos2, {});

    REQUIRE(table.find(startPos) != nullptr);
    REQUIRE(table.find(pos2) != nullptr);
    REQUIRE(table.find(startPos) != table.find(pos2));

    table.clear();

    REQUIRE(table.find(startPos) == nullptr);
    REQUIRE(table.find(pos2) == nullptr);
}

TEST_CASE("Transposition table - get_best_response()", TAGS)
{
    static const chess::game::Position startPos {};

    const auto ourMove = notation::from_alg(startPos, "Nf3");

    const auto theirMove = notation::from_alg(
        chess::game::after_move(startPos, ourMove), "Nf6");

    TranspositionTable table;

    REQUIRE(not table.get_best_response(startPos, ourMove).has_value());

    table.store(startPos,
        { .searchedDepth = 2uz,
            .eval        = 0,
            .evalType    = EvalType::Exact,
            .bestMove    = ourMove });

    REQUIRE(not table.get_best_response(startPos, ourMove).has_value());

    table.store(
        chess::game::after_move(startPos, ourMove),
        { .searchedDepth = 1uz,
            .eval        = 0,
            .evalType    = EvalType::Exact,
            .bestMove    = theirMove });

    const auto bestResponse = table.get_best_response(startPos, ourMove);

    REQUIRE(bestResponse.has_value());
    REQUIRE(*bestResponse == theirMove);
}
