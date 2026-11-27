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

#include "support/Fixtures.hpp"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <libbenbot/eval/SEE.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <sstream>
#include <string>
#include <tuner/Dataset.hpp>
#include <tuner/Resolve.hpp>

namespace {

using ben_bot::tuner::resolve;
using ben_bot::tuner::resolve_dataset;
using ben_bot::tuner::ResolveStats;
using chess::game::Position;

constexpr auto TAGS = "[tuner][resolve]";

[[nodiscard]] auto from_fen(const std::string& fen) -> Position
{
    const auto position = chess::notation::from_fen(fen);

    INFO(fen);
    REQUIRE(position.has_value());

    return *position;
}

// true if the side to move has no capture or promotion that wins material by SEE
[[nodiscard]] auto has_no_winning_captures(const Position& position) -> bool
{
    return std::ranges::none_of(chess::moves::generate(position), [&position](const auto move) {
        return not position.is_quiet(move) and ben_bot::eval::see(position, move) > 0;
    });
}

} // namespace

TEST_CASE("Resolve - quiet positions are unchanged", TAGS)
{
    for (const auto* fen : {
             "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
             "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
             "8/8/1p1k4/1P6/8/3p3P/1r4P1/5K2 w - - 0 1",
         }) {
        const auto position = from_fen(fen);

        const auto resolved = resolve(position);

        INFO(fen);
        REQUIRE(resolved.pv.empty());
        REQUIRE(resolved.position == position);
        REQUIRE(chess::notation::to_fen(resolved.position) == fen);
    }
}

TEST_CASE("Resolve - a hanging piece is taken", TAGS)
{
    const auto resolved = resolve(from_fen("4k3/8/8/3q4/8/8/8/3QK3 w - - 0 1"));

    REQUIRE(resolved.pv.size() == 1uz);
    REQUIRE(chess::notation::to_uci(resolved.pv.front()) == "d1d5");
    REQUIRE(chess::notation::to_fen(resolved.position) == "4k3/8/8/3Q4/8/8/8/4K3 b - - 0 1");
    REQUIRE(resolved.score > 0);
}

TEST_CASE("Resolve - the result has no winning captures", TAGS)
{
    // a hanging queen, an even pawn trade, a promotion, a defended pawn that isn't taken,
    // and a series of captures on one square
    for (const auto* fen : {
             "4k3/8/8/3q4/8/8/8/3QK3 w - - 0 1",
             "4k3/2n5/8/3p4/4P3/8/8/4K3 w - - 0 1",
             "8/4P3/8/8/8/k7/8/4K3 w - - 0 1",
             "4k3/8/4p3/3p4/8/8/8/3QK3 w - - 0 1",
             "3rk3/3r4/8/3p4/8/2N2N2/8/3RK3 w - - 0 1",
         }) {
        const auto position = from_fen(fen);

        const auto resolved = resolve(position);

        INFO(fen);
        INFO("resolved to " << chess::notation::to_fen(resolved.position));

        REQUIRE(not resolved.position.is_check());
        REQUIRE(has_no_winning_captures(resolved.position));
    }
}

TEST_CASE("Resolve - a resolved position resolves to itself", TAGS)
{
    // a capture that wins material by SEE can still be left at the end of the PV, if taking
    // it would let the other side take something bigger, so this is checked instead
    for (const auto& [position, result] : tuner_tests::read_positions("positions.epd")) {
        const auto resolved = resolve(position);

        INFO(chess::notation::to_fen(position));
        INFO("resolved to " << chess::notation::to_fen(resolved.position));

        // the same position always resolves the same way
        REQUIRE(resolve(position).position == resolved.position);

        if (resolved.position.is_check())
            continue;

        const auto again = resolve(resolved.position);

        REQUIRE(again.pv.empty());
        REQUIRE(again.position == resolved.position);
    }
}

TEST_CASE("Resolve - resolve_dataset keeps the labels & scores", TAGS)
{
    std::istringstream input {
        "# a comment\n"
        "4k3/8/8/3q4/8/8/8/3QK3 w - - 0 1 [1.0] 912\n"
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 [0.5]\n"
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - c9 \"0-1\";\n"
        "not a position [1.0]\n"
    };

    std::ostringstream output;

    ResolveStats stats;

    resolve_dataset(input, output, 2uz, stats);

    REQUIRE(output.str()
            == "4k3/8/8/3Q4/8/8/8/4K3 b - - 0 1 [1.0] 912\n"
               "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 [0.5]\n"
               "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 1 [0.0]\n");

    REQUIRE(stats.lines == 5uz);
    REQUIRE(stats.skipped == 1uz);
    REQUIRE(stats.parseErrors == 1uz);
    REQUIRE(stats.changed == 1uz);
    REQUIRE(stats.unchanged == 2uz);
    REQUIRE(stats.inCheck == 0uz);
}
