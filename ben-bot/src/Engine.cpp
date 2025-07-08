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
#include "Resources.hpp"
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

namespace chess {

using std::println;

namespace {
    [[nodiscard]] std::string load_file_as_string(const std::filesystem::path& file)
    {
        std::ifstream input { file };

        input.exceptions(
            std::ios_base::badbit | std::ios_base::failbit);

        using Iterator = std::istreambuf_iterator<char>;

        return { Iterator { input }, Iterator {} };
    }
} // namespace

void BenBotEngine::new_game(const bool firstCall)
{
    moves::magics::init();

    searcher.context.reset(); // clears transposition table

    if (firstCall) {
        // load embedded book data into opening book data structure
        searcher.context.openingBook.book.add_from_json(
            get_opening_book_json_text());
    }
}

// this function implements non-standard UCI commands that we support
void BenBotEngine::handle_custom_command(
    const std::string_view command, const std::string_view options)
{
    if (command == "loadbook") {
        load_book_file(std::filesystem::path { options });
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

void BenBotEngine::load_book_file(const std::filesystem::path& file)
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

void BenBotEngine::print_help() const
{
    println(
        "{} by {}",
        get_name(), get_author());

    println(
        "All standard UCI commands are supported, as well as the following non-standard commands:");

    println(
        "loadbook <path> - reads the given JSON file into the engine's openings database. See book.json in the ben-bot source code for an example of the format.");
    println(
        "options         - dump current UCI option values");
    println(
        "help            - displays this text");
}

void BenBotEngine::print_options() const
{
    const auto& ownBook = searcher.context.openingBook.enabled;

    println(
        "{} - toggle - controls whether internal opening book is used - current {} - default {}",
        ownBook.get_name(), ownBook.get_value(), ownBook.get_default_value());

    println(
        "{} - button - press to clear the transposition table",
        clearTT.get_name());
}

} // namespace chess
