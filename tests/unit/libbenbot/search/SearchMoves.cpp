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
#include <cstddef>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Options.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <string_view>

inline constexpr auto TAGS { "[search][searchmoves]" };

using ben_bot::search::Result;
using std::size_t;

namespace notation = chess::notation;

namespace {

// runs a search to the given depth & returns the depth that the search reports when it's done
[[nodiscard]] auto get_searched_depth(
    const chess::game::Position& position, const std::string_view searchMove, const size_t depth) -> size_t
{
    size_t searchedDepth { 0uz };

    ben_bot::search::Context ctx {
        ben_bot::search::Callbacks {
            .onSearchStart    = { },
            .onSearchComplete = [&searchedDepth](const Result& result) {
                searchedDepth = result.depth;
            },
            .onIteration = { },
            .onRootMove  = { } }
    };

    ctx.set_position(position);

    ben_bot::search::Options options;
    options.depth = depth;

    if (not searchMove.empty())
        options.movesToSearch.push_back(notation::from_alg(position, searchMove).value());

    ctx.set_options(options);

    // the setters above abort any in-progress search, so clear that before searching
    ctx.reset_abort();

    ctx.search();

    return searchedDepth;
}

} // namespace

TEST_CASE("Search - searchmoves with 1 move searches to the requested depth", TAGS)
{
    // the starting position has 20 legal moves
    REQUIRE(get_searched_depth(chess::game::Position { }, "e4", 4uz) == 4uz);
}

TEST_CASE("Search - only 1 legal move stops after the first iteration", TAGS)
{
    // white is in check from the rook, & Kxa2 is the only legal move
    const auto position = notation::from_fen("8/8/8/8/8/8/r7/K1k5 w - - 0 1").value();

    REQUIRE(get_searched_depth(position, { }, 4uz) == 1uz);
}
