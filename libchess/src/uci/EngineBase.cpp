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
#include <expected>
#include <format>
#include <iostream>
#include <libchess/notation/FEN.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string>
#include <string_view>

namespace chess::uci {

using printing::info_string;
using std::cout;
using std::println;
using std::string_view;
using util::split_at_first_space;
using util::trim;

// NB. With our println calls, we explicitly provide std::cout as the first parameter, because
// when the first parameter is omitted, the default overload writes to the FILE* stdout, but we
// want to use C++ iostreams for all our I/O so that our logging facility works correctly

// defined out-of-line to address -Wweak-vtables
EngineBase::~EngineBase() = default;

void EngineBase::handle_command(string_view command)
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

        println(cout, "readyok");
        cout.flush();
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
    rest      = trim(rest);

    if (firstWord == "position") {
        handle_setpos(rest);
        return;
    }

    if (firstWord == "go") {
        go(parse_go_options(rest, position));
        return;
    }

    if (firstWord == "setoption") {
        handle_setoption(rest);
        return;
    }

    if (firstWord == "debug") {
        set_debug(rest == "on");
        return;
    }

    if (firstWord == "register") {
        handle_registration(parse_register_options(rest));
        return;
    }

    handle_custom_command(firstWord, rest);
}

void EngineBase::respond_to_uci()
{
    // this command is sent once after program boot

    println(cout, "id name {}", get_name());
    println(cout, "id author {}", get_author());

    for (const auto* option : get_options())
        println(cout, "{}", option->get_declaration_string());

    println(cout, "uciok");

    cout.flush();
}

void EngineBase::handle_setpos(const string_view arguments)
{
    // According to the UCI spec, engines should ignore invalid commands.
    // If the FEN or movelist sent is invalid, we could terminate the engine
    // with an error exit code; however, it seems to be the most spec-compliant
    // behavior to ignore the invalid command and not terminate the engine.
    // If parsing returns an error, or if the new position seems to be illegal,
    // we print an error message via `info string` and keep the old position.
    // See this Stockfish PR discussion: https://github.com/official-stockfish/Stockfish/pull/4563

    using MaybeError = std::expected<void, std::string>;

    // NB. enabling this check seems to cost about 8 ELO
    static constexpr bool SanitizeIncomingPositions = false;

    [[maybe_unused]] const auto obj
        = parse_position_options(arguments)
              .and_then([this](const Position& pos) -> MaybeError {
                  if constexpr (SanitizeIncomingPositions) {
                      if (const auto errorStr = pos.is_illegal()) {
                          [[unlikely]];
                          return std::unexpected {
                              std::format("Position is illegal: {}", errorStr.value())
                          };
                      }
                  }

                  position = pos;

                  set_position(pos);

                  return {};
              })
              .or_else([this](const string_view error) -> MaybeError {
                  info_string(std::format("Error setting position: {}", error));
                  info_string(std::format("Retained previous position: {}", notation::to_fen(position)));

                  return {};
              });
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
    } else {
        info_string(std::format("Attempted to set unknown option '{}'", name));
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
