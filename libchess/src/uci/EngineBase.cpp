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

#include <algorithm>
#include <exception>
#include <iostream>
#include <libchess/notation/FEN.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string>

namespace chess::uci {

using std::println;
using util::split_at_first_space;
using util::trim;

// defined out-of-line to address -Wweak-vtables
EngineBase::~EngineBase() = default;

void EngineBase::handle_command(std::string_view command)
{
    command = trim(command);

    if (command.empty())
        return;

    if (command == "uci") {
        respond_to_uci();
        return;
    }

    if (command == "isready") {
        // reply immediately if search is in progress
        if (not is_searching())
            wait();

        println("readyok");
        return;
    }

    if (command == "ucinewgame") { // isready will be queried after this
        new_game(! initialized);
        initialized = true;
        return;
    }

    if (command == "quit") {
        abort_search();
        wait();
        shouldExit = true; // exit the event loop
        return;
    }

    if (command == "stop") {
        abort_search();
        return;
    }

    if (command == "ponderhit") {
        ponder_hit();
        return;
    }

    auto [firstWord, rest] = split_at_first_space(command);

    firstWord = trim(firstWord);

    if (firstWord == "position") {
        handle_setpos(rest);
        return;
    }

    if (firstWord == "go") {
        go(parse_go_options(rest, position));
        return;
    }

    rest = trim(rest);

    if (firstWord == "setoption") {
        handle_setoption(rest);
        return;
    }

    if (firstWord == "debug") {
        set_debug(rest == "on");
        return;
    }

    handle_custom_command(firstWord, rest);
}

void EngineBase::respond_to_uci()
{
    // this command is sent once after program boot

    println("id name {}", get_name());
    println("id author {}", get_author());

    for (const auto* option : get_options())
        println("{}", option->get_declaration_string());

    println("uciok");
}

// According to the UCI spec, engines should ignore invalid commands.
// If the FEN or movelist sent is invalid, an exception will be thrown
// when trying to parse it, which we could simply not catch here to let
// the engine terminate; however, it seems to be the most spec-compliant
// behavior to ignore the invalid command and not terminate the engine.
// If a parsing error is thrown, or if the new position is determined
// to be illegal, we print an error message via `info string` and keep
// the old position. See this Stockfish PR discussion:
// https://github.com/official-stockfish/Stockfish/pull/4563
void EngineBase::handle_setpos(const string_view arguments)
try {
    const auto newPos = parse_position_options(arguments);

    if (const auto errorStr = newPos.is_illegal()) {
        [[unlikely]];
        println("info string Attempted to set illegal position: {}", errorStr.value());
        return;
    }

    position = newPos;

    set_position(newPos);
} catch (const std::exception& exception) {
    println("info string Error setting position: {}", exception.what());
    println("info string Rolling back to previous position: {}", notation::to_fen(position));
}

void EngineBase::handle_setoption(const string_view arguments)
{
    auto [firstWord, rest] = split_at_first_space(arguments);

    firstWord = trim(firstWord);

    // code defensively against unrecognized tokens
    if (firstWord != "name") {
        [[unlikely]];
        return;
    }

    rest = trim(rest);

    // we can't just use split_at_first_space() here, because option names
    // may legally contain spaces

    const auto valueTokenIdx = rest.find("value");

    const bool isNPos = valueTokenIdx == string_view::npos;

    auto name = isNPos ? rest : rest.substr(0uz, valueTokenIdx);

    name = trim(name);

    wait();

    const auto options = get_options();

    if (const auto it = std::ranges::find_if(
            options,
            [name](const Option* opt) { return opt->get_name() == name; });
        it != options.end()) {
        auto* option = *it;

        if (isNPos)
            option->handle_setvalue({});
        else
            option->handle_setvalue(trim(rest.substr(valueTokenIdx)));

        option_changed(*option);
    }
}

void EngineBase::loop()
{
    std::string inputBuf;

    do {
        std::getline(std::cin, inputBuf);

        handle_command(inputBuf);
    } while (! shouldExit);
}

} // namespace chess::uci
