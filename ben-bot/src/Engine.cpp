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
#include <libchess/notation/PGN.hpp>
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

    void overwrite_file(path file, const string_view content)
    {
        file = absolute(file);

        // need to make sure the dest directory exists
        std::filesystem::create_directories(file.parent_path());

        std::ofstream output { file };

        output.exceptions(
            std::ios_base::badbit | std::ios_base::failbit);

        output << content;
    }
} // namespace

void Engine::new_game(const bool firstCall)
{
    searcher.context.reset(); // clears transposition table

    if (firstCall) {
        chess::moves::magics::init();

        load_book_moves(
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

void Engine::load_book_moves(const string_view pgnText)
{
    wait();

    const auto games = chess::notation::parse_all_pgns(pgnText);

    std::println("Found {} PGNs", games.size());

    for (const auto& game : games)
        searcher.context.openingBook.book.add_from_pgn(game, true);

    searcher.context.openingBook.book.print_stats();
}

void Engine::load_book_file(const path& file)
try {
    load_book_moves(
        load_file_as_string(file));
} catch (const std::exception& except) {
    std::println(std::cerr,
        "Error reading from opening book file at path: {}",
        file.string());

    std::println(std::cerr, "{}", except.what());
}

void Engine::dump_book_to_file(const path& file) const
try {
    searcher.context.wait(); // NB. the virtual wait() function is not const

    std::string fileContent;

    for (const auto& game : searcher.context.openingBook.book.to_pgns()) {
        fileContent.append(chess::notation::to_pgn(game));
        fileContent.append(2uz, '\n');
    }

    overwrite_file(file, fileContent);
} catch (const std::exception& except) {
    std::println(std::cerr,
        "Error writing opening book to file at path: {}",
        file.string());

    std::println(std::cerr, "{}", except.what());
}

void Engine::make_null_move()
{
    wait();

    searcher.context.options.position.make_null_move();
}

} // namespace ben_bot
