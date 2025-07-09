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
#include <iostream>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string>

namespace chess::uci {

using std::println;
using util::split_at_first_space;
using util::trim;

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
        position = parse_position_options(rest);
        set_position(position);
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

        options_changed();
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
