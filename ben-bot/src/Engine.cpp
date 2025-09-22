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
#include <ben-bot/Engine.hpp>
#include <chrono>
#include <cstddef>  // IWYU pragma: keep - for size_t
#include <iostream> // for cerr
#include <libchess/moves/Perft.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <utility>

namespace ben_bot {

namespace util     = chess::util;
namespace notation = chess::notation;

using std::println;
using std::size_t;

void Engine::new_game([[maybe_unused]] const bool firstCall)
{
    searcher.context.clear_transposition_table();
}

void Engine::go(uci::GoCommandOptions&& opts)
{
    searcher.start(std::move(opts));
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

void Engine::option_changed(const uci::Option& option)
{
    if (&option == &ttSize) {
        wait();
        searcher.context.transTable.resize(static_cast<size_t>(ttSize.get_value()));
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

void Engine::make_null_move()
{
    wait();

    searcher.context.options.position.make_null_move();
}

void Engine::color_flip()
{
    wait();

    searcher.context.options.position.flip();
}

} // namespace ben_bot
