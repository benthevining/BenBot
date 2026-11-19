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
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Options.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <string_view>

inline constexpr auto TAGS { "[search][MatePV]" };

using ben_bot::search::Result;
using std::size_t;

namespace notation = chess::notation;

namespace {

struct MateTestCase final {
    std::string_view fen;

    size_t bestMatePlies { 0uz }; // plies until checkmate in the fastest mating line
};

} // namespace

// Positions taken from matetrack's mates2000.epd, where the search used to report a correct
// mate score with a PV that was missing its final mating move, or contained a stale line.
// For each completed iteration that reports a mate score, the PV must be legal, have exactly
// as many moves as the mate distance, end in checkmate, and not claim a faster mate than exists.
TEST_CASE("Search - mate scores have matching PVs", TAGS)
{
    const auto testCase = GENERATE(
        // mate found in a PV node after MDP lowered beta to the exact mate score
        MateTestCase { "8/8/8/8/4Q3/8/5Rb1/R3K2k b Q - 0 1", 2uz },
        MateTestCase { "r5r1/1R4b1/1Q6/4nk1p/2p1R3/2P3PK/2BB4/q7 w - - 0 1", 3uz },
        MateTestCase { "8/7B/2R5/4Nr1p/4kb1Q/8/1B6/4K2R w K - 0 1", 3uz },
        MateTestCase { "1r4Rk/4Nq2/7K/8/8/8/b5Q1/6R1 b - - 0 1", 6uz },
        // final mating move is a capture found by the quiescence search
        MateTestCase { "3b2n1/1pp3N1/4p2p/1P1kP3/1B4N1/rpPB2p1/p3K3/8 b - - 0 1", 6uz },
        MateTestCase { "6K1/1B1NR2p/2rpP3/1p1k1p2/1R4Pn/1pB5/2N1P3/rbbq4 w - - 0 1", 7uz });

    CAPTURE(testCase.fen);

    const auto rootPosition = notation::from_fen(testCase.fen).value();

    size_t numMateResults { 0uz };

    ben_bot::search::Context ctx {
        ben_bot::search::Callbacks {
            .onSearchStart    = { },
            .onSearchComplete = { },
            .onIteration      = [&rootPosition, &testCase, &numMateResults](const Result& result) {
                if (not result.score.is_mate())
                    return;

                ++numMateResults;

                CAPTURE(result.depth);

                const auto mateDistance = result.score.ply_to_mate();

                REQUIRE(mateDistance >= testCase.bestMatePlies);
                REQUIRE(result.pv.size() == mateDistance);

                auto position = rootPosition;

                for (const auto move : result.pv) {
                    const auto legalMoves = chess::moves::generate(position);

                    REQUIRE(std::ranges::contains(legalMoves, move));

                    position.make_move(move);
                }

                REQUIRE(position.is_checkmate());
            },
            .onRootMove = { } }
    };

    ctx.set_position(rootPosition);

    ben_bot::search::Options options;
    options.depth = 6uz;

    ctx.set_options(options);

    ctx.search();

    // make sure we're actually testing something
    REQUIRE(numMateResults > 0uz);
}
