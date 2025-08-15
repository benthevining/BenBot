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

#include "Engine.hpp"
#include "Data.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <exception>
#include <iostream>
#include <libbenbot/search/Search.hpp>
#include <libchess/moves/Perft.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/util/Files.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <utility>

namespace ben_bot {

namespace util     = chess::util;
namespace notation = chess::notation;

using std::println;
using std::size_t;

void Engine::new_game(const bool firstCall)
{
    searcher.context.clear_transposition_table();

    if (firstCall) {
        searcher.context.openingBook.book.add_from_pgn(
            get_opening_book_pgn_text());
    }
}

void Engine::go(uci::GoCommandOptions&& opts)
{
    searcher.start(std::move(opts));
}

void Engine::ponder_hit()
{
    searcher.context.pondering.store(false);
}

// this function implements non-standard UCI commands that we support
void Engine::handle_custom_command(
    const string_view command, const string_view opts)
{
    if (const auto it = std::ranges::find_if(
            customCommands,
            [command](const CustomCommand& cmd) { return cmd.name == command; });
        it != customCommands.end()) {
        it->action(opts);
        return;
    }

    println(std::cerr, "Unknown UCI command: {}", command);
    println("Type 'help' for a list of supported commands");
}

void Engine::load_book_file(const string_view arguments)
{
    const auto [firstWord, rest] = util::split_at_first_space(arguments);

    const path file { firstWord };

    const bool discardVariations = util::trim(rest) == "novars";

    wait();

    try {
        searcher.context.openingBook.book.add_from_pgn(
            util::load_file_as_string(file),
            not discardVariations);
    } catch (const std::exception& except) {
        println(std::cerr,
            "Error reading from opening book file at path: {}",
            file.string()); // NOLINT(build/include_what_you_use)

        println(std::cerr, "{}", except.what());
    }
}

namespace {
    using chess::moves::PerftResult;

    void perft_print_root_nodes(const PerftResult& result)
    {
        for (const auto [move, numChildren] : result.rootNodes) {
            println("{} {}",
                notation::to_uci(move), numChildren);
        }
    }

    void perft_print_results(const PerftResult& result)
    {
        println("Nodes: {}", result.nodes);
        println("Captures: {}", result.captures);
        println("En passant captures: {}", result.enPassantCaptures);
        println("Castles: {}", result.castles);
        println("Promotions: {}", result.promotions);
        println("Checks: {}", result.checks);
        println("Checkmates: {}", result.checkmates);

        // NB. the python wrapper script relies on this being printed last
        println("Stalemates: {}", result.stalemates);
    }
} // namespace

void Engine::run_perft(const string_view arguments) const
{
    const auto depth = util::int_from_string(
        util::trim(arguments),
        4uz);

    println("Running perft depth {}...", depth);

    const auto result = chess::moves::perft(
        depth, searcher.context.options.position);

    println("");
    perft_print_root_nodes(result);
    println("");
    perft_print_results(result);
}

namespace {

    // We create a separate context to do bench searches, so that we don't need to
    // mess with changing the main searcher's callbacks, or setting an "isBench"
    // flag to be checked in the callbacks. The simplest approach is to just
    // block in this method - this function creates a search context, executes it,
    // and blocks waiting for the result.
    void do_bench(
        const string_view epdText,
        const size_t      defaultDepth)
    {
        auto totalNodes { 0uz };

        std::chrono::milliseconds totalTime { 0 };

        search::Context benchSearcher {
            search::Callbacks {
                .onSearchComplete = [&totalNodes, &totalTime](const search::Callbacks::Result& res) {
                    totalNodes += res.nodesSearched;
                    totalTime += res.duration;
                } }
        };

        auto posNum { 0uz };

        for (const auto& position : notation::parse_all_epds(epdText)) {
            benchSearcher.options.position = position.position;
            benchSearcher.options.movesToSearch.clear();

            if (const auto it = position.operations.find("depth");
                it != position.operations.end()) {
                benchSearcher.options.depth = util::int_from_string(it->second, defaultDepth);
            } else {
                benchSearcher.options.depth = defaultDepth;
            }

            println("Searching for position #{}...", posNum);

            benchSearcher.search();

            ++posNum;
        }

        const auto seconds = static_cast<double>(totalTime.count()) * 0.001;

        assert(seconds > 0.);

        const auto nps = static_cast<size_t>(std::round(
            static_cast<double>(totalNodes) / seconds));

        println("Total nodes: {}", totalNodes);
        println("NPS: {}", nps);

        println(
            R"-(<DartMeasurement name="Nodes per second" type="numeric/integer">{}</DartMeasurement>)-",
            nps);
    }

} // namespace

void Engine::run_bench(const string_view arguments)
{
    const auto [depth, filePath] = util::split_at_first_space(arguments);

    const auto defaultDepth = util::int_from_string(depth, 3uz);

    if (filePath.empty()) {
        do_bench(get_bench_epd_text(), defaultDepth);
    } else {
        do_bench(
            util::load_file_as_string(path { filePath }),
            defaultDepth);
    }
}

void Engine::make_null_move()
{
    wait();

    searcher.context.options.position.make_null_move();
}

} // namespace ben_bot
