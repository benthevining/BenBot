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

#include "Data.hpp"
#include "Engine.hpp"
#include "TextTable.hpp"
#include <format>
#include <libbenbot/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <print>
#include <string>
#include <variant>

namespace ben_bot {

using std::println;

void Engine::print_logo_and_version() const
{
    println("{}", get_ascii_logo());

    println(
        "{}, version {}, by {}",
        get_name(), get_version_string(), get_author());
}

void Engine::print_help() const
{
    print_logo_and_version();

    println();

    println(
        "All standard UCI commands are supported, as well as the following non-standard commands:");

    println();

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
    println();
    println("The following UCI options are supported:");
    println();

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

namespace {
    void print_eval(const chess::game::Position& pos)
    {
        if (pos.is_checkmate()) {
            println("eval: #");
            return;
        }

        if (pos.is_draw()) {
            println("eval: 0");
            return;
        }

        println("eval: {}", eval::evaluate(pos));
    }
} // namespace

void Engine::print_current_position() const
{
    const auto& pos = searcher.context.options.position;

    println("{}", print_utf8(pos));
    println("{}", chess::notation::to_fen(pos));
    println();
    print_eval(pos);
}

void Engine::print_compiler_info()
{
    println(
        "Compiled by {} version {} for {}",
        get_compiler_name(), get_compiler_version(), get_system_name());

    println(
        "Build configuration: {}", get_build_config());
}

} // namespace ben_bot
