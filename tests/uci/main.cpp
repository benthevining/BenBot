/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

// TODO: send "info" command to GUI

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

using chess::game::Position;
using chess::moves::Move;

using chess::notation::from_uci;
using chess::util::split_at_first_space;
using chess::util::trim;

namespace {

void print_engine_id()
{
    std::println("id name BenBot");
    std::println("id author Ben Vining");
}

} // namespace

struct UCIEngine final {
    void loop()
    {
        do {
            read_next_command();
        } while (! shouldExit);
    }

private:
    void read_next_command()
    {
        inputBuf.clear();

        std::getline(std::cin, inputBuf);

        handle_command(inputBuf);
    }

    void handle_command(std::string_view command)
    {
        command = trim(command);

        if (command.empty())
            return;

        if (command == "uci") {
            // this command is sent once after program boot
            print_engine_id();
            // print option info...
            std::println("uciok");
            return;
        }

        if (command == "isready") {
            // can block here to wait for background tasks to complete
            std::println("readyok");
            return;
        }

        if (command == "ucinewgame") {
            currentPosition = Position {};
            // flush any search-related data structures, etc.
            // isready will be queried after this
            return;
        }

        if (command == "quit") {
            shouldExit = true; // exit the event loop
            return;
        }

        if (command == "stop") {
            // interrupt the search
            return;
        }

        if (command == "ponderhit") {
            // the engine was told to ponder on the same move the user has played
            // the engine should continue searching but switch from pondering to normal search
            return;
        }

        auto [firstWord, rest] = split_at_first_space(command);

        firstWord = trim(firstWord);

        if (firstWord == "position") {
            currentPosition = chess::uci::parse_position_options(rest);
            return;
        }

        if (firstWord == "go") {
            handle_go_command(rest);
            return;
        }

        if (firstWord == "setoption") {
            handle_set_option(rest);
            return;
        }

        if (firstWord == "debug") {
            rest      = trim(rest);
            debugMode = rest == "on";
        }
    }

    void handle_go_command(const std::string_view args)
    {
        // args does not include the "go" token itself
        [[maybe_unused]] const auto opts = chess::uci::parse_go_options(args, currentPosition);

        // run search with given options...
        std::println("bestmove {}",
            chess::notation::to_uci(
                chess::search::find_best_move(currentPosition)));
    }

    void handle_set_option(const std::string_view args)
    {
        // args does not include the "setoption" token itself
    }

    Position currentPosition {};

    std::string inputBuf;

    bool shouldExit { false };

    bool debugMode { false };
};

int main(
    [[maybe_unused]] const int argc, [[maybe_unused]] const char** argv)
try {
    UCIEngine engine;

    engine.loop();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
