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
#include "TextTable.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <libchess/moves/Magics.hpp>
#include <print>
#include <string>
#include <string_view>

namespace ben_bot {

using std::println;

using Path = std::filesystem::path;

namespace {
    [[nodiscard]] std::string load_file_as_string(const Path& file)
    {
        std::ifstream input { file };

        input.exceptions(
            std::ios_base::badbit | std::ios_base::failbit);

        using Iterator = std::istreambuf_iterator<char>;

        return { Iterator { input }, Iterator {} };
    }
} // namespace

void Engine::new_game(const bool firstCall)
{
    searcher.context.reset(); // clears transposition table

    if (firstCall) {
        chess::moves::magics::init();

        // load embedded book data into opening book data structure
        searcher.context.openingBook.book.add_from_json(
            get_opening_book_json_text());
    }
}

// this function implements non-standard UCI commands that we support
void Engine::handle_custom_command(
    const string_view command, const string_view options)
{
    if (command == "loadbook") {
        load_book_file(Path { options });
        return;
    }

    if (command == "options") {
        print_options();
        return;
    }

    if (command == "help") {
        print_help();
        return;
    }

    println(std::cerr, "Unknown UCI command: {}", command);
    println("Type 'help' for a list of supported commands");
}

void Engine::load_book_file(const Path& file)
try {
    wait();

    searcher.context.openingBook.book.add_from_json(
        load_file_as_string(file));
} catch (const std::exception& except) {
    println(std::cerr,
        "Error reading from opening book file at path: {}",
        file.string());

    println(std::cerr, "{}", except.what());
}

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

    // clang-format off
    table
        .append_column("Command").append_column("Notes")
        .new_row()
        .append_column("loadbook <path>").append_column("Reads the given JSON file into the engine's openings database. See book.json in the ben-bot source code for an example of the format.")
        .new_row()
        .append_column("options").append_column("Dump current UCI option values")
        .new_row()
        .append_column("help").append_column("Display this text");
    // clang-format on

    println("{}",
        table.to_string("", "|", "\n"));
}

void Engine::print_options() const
{
    const auto& ownBook = searcher.context.openingBook.enabled;

    println(
        "{} - toggle - controls whether internal opening book is used - current {} - default {}",
        ownBook.get_name(), ownBook.get_value(), ownBook.get_default_value());

    println(
        "{} - button - press to clear the transposition table",
        clearTT.get_name());
}

} // namespace ben_bot
