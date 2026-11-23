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

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstddef>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Options.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <string_view>

inline constexpr auto TAGS { "[search][AspirationWindows]" };

using ben_bot::search::Result;
using std::size_t;

namespace notation = chess::notation;

// Searches deep enough for the aspiration window to kick in (which happens after depth 4),
// including a position where the score swings between iterations. Every completed
// iteration must report a legal, non-empty PV, and a score that isn't a window bound left over
// from a failed search.
TEST_CASE("Search - aspiration windows report valid results", TAGS)
{
    const auto fen = GENERATE(
        // starting position
        std::string_view { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" },
        // pawn endgame, where the score swings between iterations
        std::string_view { "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1" });

    CAPTURE(fen);

    const auto rootPosition = notation::from_fen(fen).value();

    size_t numResults { 0uz };

    const auto check_result = [&rootPosition, &numResults](const Result& result) {
        ++numResults;

        CAPTURE(result.depth);

        REQUIRE(not result.pv.empty());

        if (not result.score.is_mate()) {
            REQUIRE(result.score.value > -ben_bot::eval::MAX);
            REQUIRE(result.score.value < ben_bot::eval::MAX);
        }

        auto position = rootPosition;

        for (const auto move : result.pv) {
            const auto legalMoves = chess::moves::generate(position);

            REQUIRE(std::ranges::contains(legalMoves, move));

            position.make_move(move);
        }
    };

    ben_bot::search::Context ctx {
        ben_bot::search::Callbacks {
            .onSearchStart    = { },
            .onSearchComplete = check_result,
            .onIteration      = check_result,
            .onRootMove       = { } }
    };

    ctx.set_position(rootPosition);

    ben_bot::search::Options options;
    options.depth = 7uz;

    ctx.set_options(options);

    // set_options() aborts any search in progress, so the flag needs to be cleared
    ctx.reset_abort();

    ctx.search();

    // make sure we searched past the aspiration window's minimum depth
    REQUIRE(numResults >= 7uz);
}
