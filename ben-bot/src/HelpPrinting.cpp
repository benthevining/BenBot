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

namespace {
    [[nodiscard]] std::string get_help_string_internal()
    {
        TextTable table;

        table
            .append_column("Command")
            .append_column("Notes")
            .new_row()
            .append_column("loadbook <path>")
            .append_column("Reads the given JSON file into the engine's openings database. See book.json in the ben-bot source code for an example of the format.")
            .new_row()
            .append_column("showpos")
            .append_column("Prints a UTF-8 representation of the current position")
            .new_row()
            .append_column("options")
            .append_column("Dump current UCI option values")
            .new_row()
            .append_column("help")
            .append_column("Display this text");

        return table.to_string();
    }

    [[nodiscard]] string_view get_help_text()
    {
        static const auto text = get_help_string_internal();

        return text;
    }
} // namespace

void Engine::print_help() const
{
    print_logo_and_version();

    println();

    println(
        "All standard UCI commands are supported, as well as the following non-standard commands:");

    println();

    println("{}", get_help_text());
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

void Engine::print_position_utf8() const
{
    println("{}", print_utf8(searcher.context.options.position));
}

} // namespace ben_bot
