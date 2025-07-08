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

    if (command == "showpos") {
        print_position_utf8();
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

    std::println(std::cerr, "Unknown UCI command: {}", command);
    std::println("Type 'help' for a list of supported commands");
}

void Engine::load_book_file(const Path& file)
try {
    wait();

    searcher.context.openingBook.book.add_from_json(
        load_file_as_string(file));
} catch (const std::exception& except) {
    std::println(std::cerr,
        "Error reading from opening book file at path: {}",
        file.string());

    std::println(std::cerr, "{}", except.what());
}

} // namespace ben_bot
