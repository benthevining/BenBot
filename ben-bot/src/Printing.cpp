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

#include <beman/inplace_vector/inplace_vector.hpp>
#include <ben-bot/Engine.hpp>
#include <ben-bot/Resources.hpp>
#include <ben-bot/TextTable.hpp>
#include <cassert>
#include <format>
#include <iostream>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Strings.hpp>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <termcolor/termcolor.hpp>
#include <utility>
#include <variant>

namespace ben_bot {

using Result = search::Result;

using std::println;
using uci::printing::info_string;

auto Engine::get_name() const -> std::string
{
    return std::format("BenBot {}", resources::get_version_string());
}

void Engine::print_logo_and_version() const
{
    using Lines = beman::inplace_vector::inplace_vector<string_view, 11uz>;

    const auto logoLines = chess::util::lines_view(resources::get_ascii_logo())
                         | std::views::take(Lines::capacity())
                         | std::ranges::to<Lines>();

    assert(logoLines.size() == Lines::capacity());

    std::cout << termcolor::grey << logoLines.front() << '\n'
              << termcolor::blue;

    for (const auto line : logoLines | std::views::drop(1) | std::views::take(Lines::capacity() - 2uz))
        std::cout << line << '\n';

    std::cout << termcolor::grey << logoLines.back() << "\n\n"
              << termcolor::bright_white << termcolor::bold << get_name() << ", "
              << termcolor::reset << "by " << get_author() << '\n'
              << termcolor::reset;
}

namespace {
    void print_colored_table(const TextTable& table)
    {
        table.print(
            [](const string_view heading) {
                // we want the heading text to be underlined, but not the
                // whitespace that follows the text to complete the cell
                const auto trimmed = chess::util::trim(heading);

                std::cout << termcolor::bold << termcolor::underline << termcolor::bright_white
                          << trimmed
                          << termcolor::reset;

                const auto numSpaces = heading.length() - trimmed.length();

                for (auto i = 0uz; i < numSpaces; ++i)
                    std::cout << ' ';
            },
            [](const string_view cell) {
                std::cout << termcolor::bright_white << cell;
            },
            [](const string_view outline) {
                std::cout << termcolor::white << outline;
            },
            [] { std::cout << '\n'; });

        std::cout << termcolor::reset;
    }
} // namespace

void Engine::print_help(const string_view args) const
{
    const bool noLogo = [args] {
        if (args.empty())
            return false;

        return chess::util::trim(args) == "--no-logo";
    }();

    if (not noLogo) {
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

    print_colored_table(table);
}

void Engine::print_options(const string_view args) const
{
    const bool noCurrent = [args] {
        if (args.empty())
            return false;

        return chess::util::trim(args) == "--no-current";
    }();

    println("");
    println("The following UCI options are supported:");
    println("");

    TextTable table;

    table.append_column("Option")
        .append_column("Type")
        .append_column("Notes")
        .append_column("Default");

    if (not noCurrent)
        table.append_column("Current");

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

            if (not noCurrent) {
                std::visit(
                    [&table](auto value) {
                        table.append_column(std::format("{}", value));
                    },
                    option->get_value_variant());
            }
        }
    }

    print_colored_table(table);

    println("");

    if (not noCurrent)
        println("Debug mode: {}", debugMode.load());
}

namespace {
    void print_colored_board(const Position& pos, const bool utf8)
    {
        const auto boardStr = utf8 ? print_utf8(pos) : print_ascii(pos);

        using Lines = beman::inplace_vector::inplace_vector<string_view, 9uz>;

        const auto lines = chess::util::lines_view(boardStr)
                         | std::ranges::to<Lines>();

        assert(lines.size() == Lines::capacity());

        for (const auto line : lines | std::views::take(Lines::capacity() - 1uz)) {
            std::cout << termcolor::bright_white << line.substr(0uz, line.length() - 1uz)
                      << termcolor::white << line.back() << '\n';
        }

        std::cout << termcolor::white << lines.back() << '\n'
                  << termcolor::reset;
    }

    void print_labeled_info(const string_view label, const string_view info)
    {
        std::cout << termcolor::white << label << termcolor::bright_white << info << '\n'
                  << termcolor::reset;
    }
} // namespace

void Engine::print_current_position(const string_view arguments) const
{
    const auto& pos = searcher.context.options.position;

    print_colored_board(pos,
        chess::util::trim(arguments) == "utf8");

    println("");

    print_labeled_info("FEN: ", chess::notation::to_fen(pos));

    print_labeled_info("Zobrist key: ", std::format("{}", pos.hash));

    print_labeled_info("Static eval: ", std::format("{}", eval::evaluate(pos)));

    searcher.context.transTable.find(pos)
        .transform([](const TTData& data) {
            print_labeled_info(
                "TT hit: ",
                std::format(
                    "depth {} eval {} type {} probed {} bestmove {}",
                    data.searchedDepth, data.eval,
                    magic_enum::enum_name(data.evalType),
                    eval::Score::from_tt(data.eval, 0uz),
                    chess::notation::to_uci(data.bestMove.value_or(Move {}))));

            return std::monostate {};
        });
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
