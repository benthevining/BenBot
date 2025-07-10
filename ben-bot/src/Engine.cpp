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
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string>
#include <string_view>

namespace ben_bot {

namespace {
    [[nodiscard]] std::string load_file_as_string(path file)
    {
        file = absolute(file);

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

        searcher.context.openingBook.book.add_from_pgn(
            get_opening_book_pgn_text());
    }
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

    std::println(std::cerr, "Unknown UCI command: {}", command);
    std::println("Type 'help' for a list of supported commands");
}

void Engine::load_book_file(const string_view arguments)
{
    const auto [firstWord, rest] = chess::util::split_at_first_space(arguments);

    const path file { firstWord };

    const bool discardVariations = chess::util::trim(rest) == "novars";

    wait();

    try {
        searcher.context.openingBook.book.add_from_pgn(
            load_file_as_string(file),
            ! discardVariations);
    } catch (const std::exception& except) {
        std::println(std::cerr,
            "Error reading from opening book file at path: {}",
            file.string());

        std::println(std::cerr, "{}", except.what());
    }
}

void Engine::make_null_move()
{
    wait();

    searcher.context.options.position.make_null_move();
}

} // namespace ben_bot
