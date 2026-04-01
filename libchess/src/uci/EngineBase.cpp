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
#include <array>
#include <atomic>
#include <cassert>
#include <expected>
#include <format>
#include <iostream>
#include <libchess/notation/FEN.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Options.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Strings.hpp>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace chess::uci {

using printing::info_string;
using std::cout;
using std::memory_order_acquire;
using std::memory_order_relaxed;
using std::memory_order_release;
using std::println;
using std::string_view;
using util::strings::split_at_first_space;
using util::strings::trim;

// NB. With our println calls, we explicitly provide std::cout as the first parameter, because
// when the first parameter is omitted, the default overload writes to the FILE* stdout, but we
// want to use C++ iostreams for all our I/O so that our logging facility works correctly

// defined out-of-line to address -Wweak-vtables
EngineBase::~EngineBase() = default;

namespace {
    // returns name of closest known command
    [[nodiscard]] auto find_nearest_command(
        const string_view input, const EngineBase::CommandList standardCommands, const EngineBase::CommandList customCommands)
        -> string_view
    {
        // map commands to pair of: command name, Levenshtein distance from input
        const auto mapped
            = std::views::join(std::array { standardCommands, customCommands })
            | std::views::transform([input](const EngineCommand& command) {
                  return std::make_pair(
                      command.name,
                      util::strings::levenshtein_distance(input, command.name));
              })
            | std::ranges::to<std::vector>();

        const auto closest = std::ranges::min(
            mapped, std::ranges::less { }, [](const auto& item) { return item.second; });

        return closest.first;
    }
} // namespace

void EngineBase::handle_command(const string_view command)
{
    auto [firstWord, rest] = split_at_first_space(command);

    firstWord = trim(firstWord);
    rest      = trim(rest);

    if (firstWord.empty())
        return;

    if (const auto it = std::ranges::find(standardUCICommands, firstWord, &EngineCommand::name);
        it != standardUCICommands.end()) {
        it->action(rest);
        return;
    }

    const auto customCommands = get_custom_uci_commands();

    if (const auto it = std::ranges::find(customCommands, firstWord, &EngineCommand::name);
        it != customCommands.end()) {
        it->action(rest);
        return;
    }

    info_string(std::format(
        "Unknown UCI command: '{}'", firstWord));

    info_string(std::format(
        "The closest known command is: {}",
        find_nearest_command(firstWord, standardUCICommands, customCommands)));
}

void EngineBase::respond_to_uci()
{
    // this command is sent once after program boot

    println(cout, "id name {}", get_name());
    println(cout, "id author {}", get_author());

    for (const auto* option : standardUCIOptions)
        println(cout, "{}", option->get_declaration_string());

    for (const auto* option : get_custom_uci_options())
        println(cout, "{}", option->get_declaration_string());

    println(cout, "uciok");

    cout.flush();
}

void EngineBase::respond_to_isready()
{
    // reply immediately if search is in progress
    // if not searching, wait on any background tasks before replying
    if (not is_searching())
        wait();

    println(cout, "readyok");

    cout.flush();
}

void EngineBase::respond_to_newgame()
{
    const bool wasInitialized = initialized.exchange(true, memory_order_relaxed);

    new_game(not wasInitialized);
}

void EngineBase::handle_quit()
{
    abort_search();

    shouldExit.store(true, memory_order_release);

    wait();
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

    [[maybe_unused]] const auto obj
        = parse_position_options(arguments)
              .and_then([this](const Position& pos) -> std::expected<void, std::string> {
                  if (sanitizeIncomingPositions.load(memory_order_acquire)) {
                      if (const auto errorStr = pos.is_illegal()) {
                          [[unlikely]];
                          return std::unexpected {
                              std::format("Position is illegal: {}", errorStr.value())
                          };
                      }
                  }

                  position = pos;

                  set_position(pos);

                  return { };
              })
              .transform_error([this](const string_view error) {
                  info_string(std::format("Error setting position: {}", error));
                  info_string(std::format("Retained previous position: {}", notation::to_fen(position)));

                  return std::monostate { };
              });
}

namespace {
    // returns name of closest known option
    [[nodiscard]] auto find_nearest_option(
        const string_view input, const EngineBase::OptionList standardOptions, const EngineBase::OptionList customOptions)
        -> string_view
    {
        // map options to pair of: option name, Levenshtein distance from input
        const auto mapped
            = std::views::join(std::array { standardOptions, customOptions })
            | std::views::transform([input](const Option* option) {
                  return std::make_pair(
                      option->get_name(),
                      util::strings::levenshtein_distance(input, option->get_name()));
              })
            | std::ranges::to<std::vector>();

        const auto closest = std::ranges::min(
            mapped, std::ranges::less { }, [](const auto& item) { return item.second; });

        return closest.first;
    }
} // namespace

void EngineBase::handle_setoption(const string_view arguments)
{
    auto [firstWord, rest] = split_at_first_space(arguments);

    firstWord = trim(firstWord);

    // code defensively against unrecognized tokens
    if (firstWord != "name")
        return;

    rest = trim(rest);

    // we can't just use split_at_first_space() here, because option names
    // may legally contain spaces

    const auto valueTokenIdx = rest.find("value");

    const bool isNPos = valueTokenIdx == string_view::npos;

    auto name = isNPos ? rest : rest.substr(0uz, valueTokenIdx);

    name = trim(name);

    wait();

    auto update_option = [name, isNPos, rest, valueTokenIdx](const OptionList options) {
        if (const auto it = std::ranges::find_if(
                options,
                [name](const Option* opt) { return opt->get_name() == name; });
            it != options.end()) {
            auto* option = *it;

            assert(option != nullptr);

            if (isNPos)
                option->handle_setvalue({ });
            else
                option->handle_setvalue(trim(rest.substr(valueTokenIdx)));

            return true;
        }

        return false;
    };

    if (update_option(standardUCIOptions))
        return;

    if (update_option(get_custom_uci_options()))
        return;

    info_string(std::format(
        "Attempted to set unknown option '{}'", name));

    info_string(std::format(
        "The closest known option is: {}",
        find_nearest_option(name, standardUCIOptions, get_custom_uci_options())));
}

void EngineBase::loop()
{
    std::string inputBuf;

    do {
        std::getline(std::cin, inputBuf);

        handle_command(inputBuf);
    } while (not shouldExit.load(memory_order_acquire));
}

} // namespace chess::uci
