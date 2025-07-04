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

#include <iostream>
#include <libchess/moves/Magics.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string>

namespace chess::uci {

using std::println;
using util::split_at_first_space;
using util::trim;

EngineBase::EngineBase()
{
    moves::magics::init();
}

void EngineBase::handle_command(std::string_view command)
{
    command = trim(command);

    if (command.empty())
        return;

    if (command == "uci") { // this command is sent once after program boot
        println("id name {}", get_name());
        println("id author {}", get_author());

        for (const auto* option : get_options())
            println("{}", option->get_declaration_string());

        println("uciok");
        return;
    }

    if (command == "isready") {
        wait();
        println("readyok");
        return;
    }

    if (command == "ucinewgame") { // isready will be queried after this
        new_game();
        return;
    }

    if (command == "quit") {
        abort_search();
        wait();
        shouldExit = true; // exit the event loop
        return;
    }

    if (command == "stop") {
        stop_search();
        return;
    }

    if (command == "ponderhit") {
        ponder_hit();
        return;
    }

    auto [firstWord, rest] = split_at_first_space(command);

    firstWord = trim(firstWord);

    if (firstWord == "position") {
        position = parse_position_options(rest);
        set_position(position);
        return;
    }

    if (firstWord == "go") {
        go(parse_go_options(rest, position));
        return;
    }

    if (firstWord == "setoption") {
        for (auto* option : get_options())
            option->parse(rest);

        options_changed();
        return;
    }

    if (firstWord == "debug") {
        rest = trim(rest);

        set_debug(rest == "on");
    }
}

void EngineBase::loop()
{
    std::string inputBuf;

    do {
        inputBuf.clear();

        std::getline(std::cin, inputBuf);

        handle_command(inputBuf);
    } while (! shouldExit);
}

} // namespace chess::uci
