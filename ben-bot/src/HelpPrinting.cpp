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
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <print>
#include <string>

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
    println("The following UCI options are supported:");

    println();

    TextTable table;

    table.append_column("Option")
        .append_column("Type")
        .append_column("Notes")
        .append_column("Default")
        .append_column("Current");

    const auto& ownBook = searcher.context.openingBook.enabled;

    table.new_row()
        .append_column(ownBook.get_name())
        .append_column("Toggle")
        .append_column("Controls whether internal opening book is used")
        .append_column(std::format("{}", ownBook.get_default_value()))
        .append_column(std::format("{}", ownBook.get_value()));

    table.new_row()
        .append_column(clearTT.get_name())
        .append_column("Button")
        .append_column("Press to clear the transposition table");

    println("{}", table.to_string());
}

void Engine::print_current_position() const
{
    const auto& pos = searcher.context.options.position;

    println("{}", print_utf8(pos));
    println("{}", chess::notation::to_fen(pos));
}

} // namespace ben_bot
