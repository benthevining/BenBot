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
#include <chrono>
#include <cstddef>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Constants.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Options.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/notation/FEN.hpp>

inline constexpr auto TAGS { "[search][depth]" };

using ben_bot::search::Result;
using std::size_t;

TEST_CASE("Search - never searches deeper than MAX_DEPTH", TAGS)
{
    // White is mated in 1 & has 3 legal moves, so each iteration only searches a few
    // nodes, and a search with no depth limit would keep going far past MAX_PLY
    const auto position = chess::notation::from_fen("8/K3q1p1/4k2p/7P/1p1n4/8/8/5q2 w - - 4 63").value();

    size_t searchedDepth { 0uz };
    size_t deepestIteration { 0uz };

    ben_bot::search::Context ctx {
        ben_bot::search::Callbacks {
            .onSearchStart    = { },
            .onSearchComplete = [&searchedDepth](const Result& result) { searchedDepth  = result.depth; },
            .onIteration = [&deepestIteration](const Result& result) { deepestIteration = std::max(deepestIteration, result.depth); },
            .onRootMove = { } }
    };

    ctx.set_position(position);

    ben_bot::search::Options options;
    options.depth = 1000uz;

    // so that this test fails instead of taking forever if an iteration gets too deep
    options.searchTime = std::chrono::seconds { 10 };

    ctx.set_options(options);

    // the setters above abort any in-progress search, so clear that before searching
    ctx.reset_abort();

    ctx.search();

    REQUIRE(searchedDepth == ben_bot::search::MAX_DEPTH);
    REQUIRE(deepestIteration < ben_bot::search::MAX_DEPTH);
}
