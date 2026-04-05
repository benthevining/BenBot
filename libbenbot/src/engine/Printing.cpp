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

#include <format>
#include <iostream>
#include <libbenbot/Resources.hpp>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/engine/ColorPrinting.hpp>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Strings.hpp>
#include <libutil/TextTable.hpp>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <string>
#include <string_view>
#include <variant>

namespace ben_bot {

using std::println;
using uci::printing::info_string;
using util::strings::trim;

using Result = search::Result;
using util::strings::TextTable;

auto Engine::get_name() const -> std::string
{
    return std::format("BenBot {}", resources::get_version_string());
}

namespace {
    void print_command_table(const uci::EngineBase::CommandList commands)
    {
        TextTable table;

        table.append_column("Command")
            .append_column("Notes");

        for (const auto& command : commands) {
            table.new_row()
                .append_column(std::format("{} {}", command.name, command.argsHelp))
                .append_column(command.description);
        }

        print_colored_table(table);

        std::cout.flush();
    }
} // namespace

void Engine::print_help(const string_view args) const
{
    const bool noLogo = [args] {
        if (args.empty())
            return false;

        return trim(args) == "--no-logo";
    }();

    if (not noLogo) {
        print_logo_and_version();

        println("");
    }

    println("The following standard UCI commands are supported:");
    println("");

    print_command_table(get_standard_uci_commands());
    println("");

    println("The following non-standard UCI commands are supported:");
    println("");

    print_command_table(customCommands);
}

void Engine::print_options()
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

    auto add_options = [&table](const OptionList opts) {
        for (const auto* option : opts) {
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
    };

    add_options(get_standard_uci_options());
    add_options(options);

    print_colored_table(table);

    println("");
    println("Debug mode: {}", is_debug_mode());

    std::cout.flush();
}

void Engine::print_current_position(const string_view arguments) const
{
    const auto& pos = searcher.context.get_position();

    print_colored_board(pos,
        trim(arguments) == "utf8");

    println("");

    print_labeled_info("FEN: ", chess::notation::to_fen(pos));

    print_labeled_info("Zobrist key: ", std::format("{}", pos.hash));

    print_labeled_info("Static eval: ", std::format("{}", eval::evaluate(pos)));

    searcher.context.probe_transposition_table(pos)
        .transform([this, &pos](const TTData& data) {
            print_labeled_info(
                "TT hit: ",
                std::format(
                    "depth {} eval {} type {} probed {} bestmove {}",
                    data.searchedDepth, data.eval,
                    magic_enum::enum_name(data.evalType),
                    eval::Score::from_tt(data.eval, 0uz),
                    pretty_print_move(
                        data.bestMove.value_or(Move { }), pos)));

            return std::monostate { };
        });

    std::cout.flush();
}

void Engine::print_compiler_info()
{
    info_string(std::format(
        "Compiled by {}, version {}, for {}",
        resources::get_compiler_name(),
        resources::get_compiler_version(),
        resources::get_system_name()));

    info_string(std::format(
        "Build configuration: {}",
        resources::get_build_config()));
}

auto Engine::pretty_print_move(
    const Move move, const Position& position) const -> std::string
{
    return format_move(
        get_pretty_print_move_format(),
        position, move,
        algFormatUTF8PieceType.get_value());
}

} // namespace ben_bot
