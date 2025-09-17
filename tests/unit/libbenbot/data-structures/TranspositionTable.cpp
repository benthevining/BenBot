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
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

static constexpr auto TAGS { "[data-structures][TranspositionTable]" };

using ben_bot::TranspositionTable;
using chess::game::Position;
using EvalType = TranspositionTable::Record::EvalType;

namespace notation = chess::notation;

TEST_CASE("Transposition table - find()", TAGS)
{
    static const Position startPos {};

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
    static const Position startPos {};

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

TEST_CASE("Transposition table - probe_eval()", TAGS)
{
    static const Position startPos {};

    static constexpr auto DEPTH = 2uz;
    static constexpr auto ALPHA = -2;
    static constexpr auto BETA  = 2;
    static constexpr auto EVAL  = 1;

    TranspositionTable table;

    REQUIRE(not table.probe_eval(
                         startPos, DEPTH, ALPHA, BETA)
            .has_value());

    SECTION("Exact eval stored")
    {
        table.store(startPos,
            { .searchedDepth = DEPTH,
                .eval        = EVAL,
                .evalType    = EvalType::Exact });

        const auto probed = table.probe_eval(startPos, DEPTH, ALPHA, BETA);

        REQUIRE(probed.has_value());

        const auto [eval, type] = probed.value();

        REQUIRE(type == EvalType::Exact);
        REQUIRE(eval == EVAL);
    }

    SECTION("Alpha cutoff stored")
    {
        table.store(startPos,
            { .searchedDepth = DEPTH,
                .eval        = EVAL,
                .evalType    = EvalType::Alpha });

        SECTION("Probing with alpha that doesn't cutoff")
        {
            static constexpr auto SEARCH_ALPHA = EVAL + 1;

            const auto probed = table.probe_eval(startPos, DEPTH, SEARCH_ALPHA, BETA);

            REQUIRE(probed.has_value());

            const auto [eval, type] = probed.value();

            REQUIRE(type == EvalType::Alpha);
            REQUIRE(eval == SEARCH_ALPHA);
        }

        SECTION("Probing with alpha that causes cutoff")
        {
            REQUIRE(not table.probe_eval(
                                 startPos, DEPTH, EVAL - 1, BETA)
                    .has_value());
        }
    }

    SECTION("Beta cutoff stored")
    {
        table.store(startPos,
            { .searchedDepth = DEPTH,
                .eval        = EVAL,
                .evalType    = EvalType::Beta });

        SECTION("Probing with beta that doesn't cutoff")
        {
            static constexpr auto SEARCH_BETA = EVAL - 1;

            const auto probed = table.probe_eval(startPos, DEPTH, ALPHA, SEARCH_BETA);

            REQUIRE(probed.has_value());

            const auto [eval, type] = probed.value();

            REQUIRE(type == EvalType::Beta);
            REQUIRE(eval == SEARCH_BETA);
        }

        SECTION("Probing with beta that causes cutoff")
        {
            REQUIRE(not table.probe_eval(
                                 startPos, DEPTH, ALPHA, EVAL + 1)
                    .has_value());
        }
    }
}

TEST_CASE("Transposition table - store() overwriting rules", TAGS)
{
    using Record = TranspositionTable::Record;

    static const Position startPos {};

    TranspositionTable table;

    const Record oldRecord {
        .searchedDepth = 6uz,
        .eval          = 2,
        .evalType      = EvalType::Exact
    };

    table.store(startPos, oldRecord);

    REQUIRE(*table.find(startPos) == oldRecord);

    SECTION("Old eval kept if it's a greater depth than the new one")
    {
        const Record newRecord {
            .searchedDepth = oldRecord.searchedDepth - 1uz,
            .eval          = 4,
            .evalType      = EvalType::Exact
        };

        table.store(startPos, newRecord);

        REQUIRE(*table.find(startPos) == oldRecord);
    }

    SECTION("Old eval kept if it was an exact one & the new one isn't")
    {
        SECTION("Writing an alpha cutoff")
        {
            const Record newRecord {
                .searchedDepth = oldRecord.searchedDepth,
                .eval          = -6,
                .evalType      = EvalType::Alpha
            };

            table.store(startPos, newRecord);

            REQUIRE(*table.find(startPos) == oldRecord);
        }

        SECTION("Writing a beta cutoff")
        {
            const Record newRecord {
                .searchedDepth = oldRecord.searchedDepth,
                .eval          = 6,
                .evalType      = EvalType::Beta
            };

            table.store(startPos, newRecord);

            REQUIRE(*table.find(startPos) == oldRecord);
        }
    }
}
