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
#include <libbenbot/search/Bounds.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>

inline constexpr auto TAGS { "[data-structures][TranspositionTable]" };

using ben_bot::TranspositionTable;
using ben_bot::TTData;
using chess::game::Position;
using EvalType = ben_bot::EvalType;

namespace notation = chess::notation;

static const Position startPos { };

TEST_CASE("Transposition table - find()", TAGS)
{
    static constexpr TTData record {
        .searchedDepth = 1uz,
        .eval          = 0,
        .evalType      = EvalType::Exact,
        .bestMove      = { }
    };

    const auto pos2 = notation::from_fen("8/8/4n3/2B1k1p1/3Pn3/2K5/5R2/8 b - - 0 1")
                          .value();

    TranspositionTable table;

    REQUIRE_FALSE(table.find(startPos));
    REQUIRE_FALSE(table.find(pos2));

    table.store(startPos, record);

    REQUIRE(table.find(startPos));
    REQUIRE_FALSE(table.find(pos2));

    table.store(pos2, record);

    REQUIRE(table.find(startPos));
    REQUIRE(table.find(pos2));

    table.clear();

    REQUIRE_FALSE(table.find(startPos));
    REQUIRE_FALSE(table.find(pos2));
}

TEST_CASE("Transposition table - get_best_response()", TAGS)
{
    const auto ourMove = notation::from_alg(startPos, "Nf3").value();

    const auto theirMove = notation::from_alg(
        chess::game::after_move(startPos, ourMove), "Nf6")
                               .value();

    TranspositionTable table;

    REQUIRE_FALSE(table.get_best_response(startPos, ourMove));

    table.store(startPos,
        { .searchedDepth = 2uz,
            .eval        = 0,
            .evalType    = EvalType::Exact,
            .bestMove    = ourMove });

    REQUIRE_FALSE(table.get_best_response(startPos, ourMove));

    table.store(
        chess::game::after_move(startPos, ourMove),
        { .searchedDepth = 1uz,
            .eval        = 0,
            .evalType    = EvalType::Exact,
            .bestMove    = theirMove });

    const auto bestResponse = table.get_best_response(startPos, ourMove);

    REQUIRE(bestResponse);
    REQUIRE(*bestResponse == theirMove);
}

TEST_CASE("Transposition table - probe_eval()", TAGS)
{
    static constexpr auto DEPTH = 2uz;
    static constexpr auto ALPHA = -2;
    static constexpr auto BETA  = 2;
    static constexpr auto EVAL  = 1;

    static constexpr ben_bot::search::Bounds BOUNDS {
        .alpha = { ALPHA },
        .beta  = { BETA }
    };

    TranspositionTable table;

    REQUIRE_FALSE(table.probe_eval(startPos, DEPTH, BOUNDS));

    SECTION("Exact eval stored")
    {
        table.store(startPos,
            { .searchedDepth = DEPTH,
                .eval        = EVAL,
                .evalType    = EvalType::Exact,
                .bestMove    = { } });

        const auto probed = table.probe_eval(startPos, DEPTH, BOUNDS);

        REQUIRE(probed);

        const auto [eval, type] = probed.value();

        REQUIRE(type == EvalType::Exact);
        REQUIRE(eval == EVAL);
    }

    SECTION("Alpha cutoff stored")
    {
        table.store(startPos,
            { .searchedDepth = DEPTH,
                .eval        = EVAL,
                .evalType    = EvalType::Alpha,
                .bestMove    = { } });

        SECTION("Probing with alpha that doesn't cutoff")
        {
            static constexpr auto SEARCH_ALPHA = EVAL + 1;

            auto thisBounds  = BOUNDS;
            thisBounds.alpha = { SEARCH_ALPHA };

            const auto probed = table.probe_eval(startPos, DEPTH, thisBounds);

            REQUIRE(probed);

            const auto [eval, type] = probed.value();

            REQUIRE(type == EvalType::Alpha);
            REQUIRE(eval == SEARCH_ALPHA);
        }

        SECTION("Probing with alpha that causes cutoff")
        {
            auto thisBounds  = BOUNDS;
            thisBounds.alpha = { EVAL - 1 };

            REQUIRE_FALSE(table.probe_eval(startPos, DEPTH, thisBounds));
        }
    }

    SECTION("Beta cutoff stored")
    {
        table.store(startPos,
            { .searchedDepth = DEPTH,
                .eval        = EVAL,
                .evalType    = EvalType::Beta,
                .bestMove    = { } });

        SECTION("Probing with beta that doesn't cutoff")
        {
            static constexpr auto SEARCH_BETA = EVAL - 1;

            auto thisBounds = BOUNDS;
            thisBounds.beta = { SEARCH_BETA };

            const auto probed = table.probe_eval(startPos, DEPTH, thisBounds);

            REQUIRE(probed);

            const auto [eval, type] = probed.value();

            REQUIRE(type == EvalType::Beta);
            REQUIRE(eval == SEARCH_BETA);
        }

        SECTION("Probing with beta that causes cutoff")
        {
            auto thisBounds = BOUNDS;
            thisBounds.beta = { EVAL + 1 };

            REQUIRE_FALSE(table.probe_eval(startPos, DEPTH, thisBounds));
        }
    }
}

TEST_CASE("Transposition table - store() overwriting rules", TAGS)
{
    TranspositionTable table;

    static constexpr TTData oldRecord {
        .searchedDepth = 6uz,
        .eval          = 2,
        .evalType      = EvalType::Exact,
        .bestMove      = { }
    };

    table.store(startPos, oldRecord);

    REQUIRE(*table.find(startPos) == oldRecord);

    SECTION("Old eval kept if it's a greater depth than the new one")
    {
        static constexpr TTData newRecord {
            .searchedDepth = oldRecord.searchedDepth - 1uz,
            .eval          = 4,
            .evalType      = EvalType::Exact,
            .bestMove      = { }
        };

        table.store(startPos, newRecord);

        REQUIRE(*table.find(startPos) == oldRecord);
    }

    SECTION("Old eval kept if it was an exact one & the new one isn't")
    {
        SECTION("Writing an alpha cutoff")
        {
            static constexpr TTData newRecord {
                .searchedDepth = oldRecord.searchedDepth,
                .eval          = -6,
                .evalType      = EvalType::Alpha,
                .bestMove      = { }
            };

            table.store(startPos, newRecord);

            REQUIRE(*table.find(startPos) == oldRecord);
        }

        SECTION("Writing a beta cutoff")
        {
            static constexpr TTData newRecord {
                .searchedDepth = oldRecord.searchedDepth,
                .eval          = 6,
                .evalType      = EvalType::Beta,
                .bestMove      = { }
            };

            table.store(startPos, newRecord);

            REQUIRE(*table.find(startPos) == oldRecord);
        }
    }
}
