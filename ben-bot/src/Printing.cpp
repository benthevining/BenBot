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

#include <ben-bot/Engine.hpp>
#include <ben-bot/Resources.hpp>
#include <ben-bot/TextTable.hpp>
#include <format>
#include <iostream>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Strings.hpp>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <string>
#include <string_view>
#include <variant>

namespace ben_bot {

using Result = search::Result;

using std::println;
using uci::printing::info_string;

namespace {

    template <bool PrintBestMove>
    void print_uci_info(
        const Result& res, const bool debugMode,
        const search::Context& context)
    {
        uci::printing::search_info(res.to_libchess(debugMode));

        if constexpr (PrintBestMove) {
            const auto& currPos    = context.options.position;
            const auto& transTable = context.transTable;

            uci::printing::best_move(
                res.bestMove,
                transTable.get_best_response(currPos, res.bestMove));

            // Because these callbacks are executed on the searcher background thread,
            // without this flush here, the output may not actually be written when we
            // expect, leading to timeouts or GUIs thinking we've hung/disconnected.
            // Because the best move is always printed last after all info output, we
            // can do the flush only in this branch.
            std::cout.flush();
        }
    }

} // namespace

Engine::Engine()
    : searcher {
        search::Callbacks {
            .onSearchComplete = [this](const Result& res) { print_uci_info<true>(res, debugMode.load(), searcher.context); },
            .onIteration = [this](const Result& res) { print_uci_info<false>(res, debugMode.load(), searcher.context); } }
    }
{
}

std::string Engine::get_name() const
{
    return std::format("BenBot {}", resources::get_version_string());
}

void Engine::print_logo_and_version() const
{
    println("{}", resources::get_ascii_logo());

    println(
        "{}, version {}, by {}",
        get_name(), resources::get_version_string(), get_author());
}

void Engine::print_help(const string_view args) const
{
    const bool noLogo = [args] {
        if (args.empty())
            return false;

        return chess::util::trim(args) == "--no-logo";
    }();

    if (! noLogo) {
        print_logo_and_version();

        println("");
    }

    println(
        "All standard UCI commands are supported, as well as the following non-standard commands:");

    println("");

    TextTable table;

    table.append_column("Command")
        .append_column("Notes");

    for (const auto& command : customCommands) {
        table.new_row()
            .append_column(std::format("{} {}", command.name, command.argsHelp))
            .append_column(command.description);
    }

    println("{}", table.to_string());
}

void Engine::print_options() const
{
    println("");
    println("The following UCI options are supported:");
    println("");

    TextTable table;

    table.append_column("Option")
        .append_column("Type")
        .append_column("Notes")
        .append_column("Default")
        .append_column("Current");

    for (const auto* option : options) {
        table.new_row()
            .append_column(option->get_name())
            .append_column(option->get_type())
            .append_column(option->get_help());

        if (option->has_value()) {
            std::visit(
                [&table](auto defaultValue) {
                    table.append_column(std::format("{}", defaultValue));
                },
                option->get_default_value_variant());

            std::visit(
                [&table](auto value) {
                    table.append_column(std::format("{}", value));
                },
                option->get_value_variant());
        }
    }

    println("{}", table.to_string());
}

void Engine::print_current_position(const string_view arguments) const
{
    const auto& pos = searcher.context.options.position;

    const bool utf8 = chess::util::trim(arguments) == "utf8";

    println("{}",
        utf8 ? print_utf8(pos) : print_ascii(pos));

    println("");
    info_string(std::format("FEN: {}", chess::notation::to_fen(pos)));
    info_string(std::format("Zobrist key: {}", pos.hash));

    if (const auto record = searcher.context.transTable.find(pos)) {
        const auto score = eval::Score::from_tt({ record->eval, record->evalType }, 0uz);

        info_string(std::format(
            "TT hit: depth {} eval {} type {} probed {} bestmove {}",
            record->searchedDepth, record->eval,
            magic_enum::enum_name(record->evalType),
            score,
            chess::notation::to_uci(record->bestMove.value_or(Move {}))));
    }

    info_string(std::format("Static eval: {}", eval::evaluate(pos)));
}

void Engine::print_compiler_info()
{
    info_string(std::format(
        "Compiled by {} version {} for {}",
        resources::get_compiler_name(),
        resources::get_compiler_version(),
        resources::get_system_name()));

    info_string(std::format(
        "Build configuration: {}",
        resources::get_build_config()));

    info_string(std::format(
        "Build date: {}",
        resources::get_build_time()));
}

} // namespace ben_bot
