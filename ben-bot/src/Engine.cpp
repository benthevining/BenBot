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
#include <exception>
#include <iostream>
#include <libchess/moves/Perft.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/util/Files.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <utility>

namespace ben_bot {

using std::println;

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

// this function implements non-standard UCI commands that we support
void Engine::handle_custom_command(
    const string_view command, const string_view options)
{
    if (const auto it = std::ranges::find_if(
            customCommands,
            [command](const CustomCommand& cmd) { return cmd.name == command; });
        it != customCommands.end()) {
        it->action(options);
        return;
    }

    println(std::cerr, "Unknown UCI command: {}", command);
    println("Type 'help' for a list of supported commands");
}

void Engine::load_book_file(const string_view arguments)
{
    const auto [firstWord, rest] = chess::util::split_at_first_space(arguments);

    const path file { firstWord };

    const bool discardVariations = chess::util::trim(rest) == "novars";

    wait();

    try {
        searcher.context.openingBook.book.add_from_pgn(
            chess::util::load_file_as_string(file),
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
                chess::notation::to_uci(move), numChildren);
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
        println("Stalemates: {}", result.stalemates);
    }
} // namespace

void Engine::run_perft(const string_view arguments) const
{
    const auto depth = chess::util::int_from_string(
        chess::util::trim(arguments),
        4uz);

    println("Running perft depth {}...", depth);

    const auto result = chess::moves::perft(
        depth, searcher.context.options.position);

    println();
    perft_print_root_nodes(result);
    println();
    perft_print_results(result);
}

void Engine::run_bench(string_view arguments)
{
    arguments = chess::util::trim(arguments);

    if (arguments.empty()) {
        do_bench(get_bench_epd_text());
    } else {
        do_bench(
            chess::util::load_file_as_string(path { arguments }));
    }
}

void Engine::do_bench(const string_view epdText)
{
    const auto positions = chess::notation::parse_all_epds(epdText);
}

void Engine::make_null_move()
{
    wait();

    searcher.context.options.position.make_null_move();
}

} // namespace ben_bot
