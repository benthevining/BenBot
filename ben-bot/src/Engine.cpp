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
#include <atomic>
#include <ben-bot/Engine.hpp>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <filesystem>
#include <format>
#include <libbenbot/search/Callbacks.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Logger.hpp>
#include <string>

namespace ben_bot {

using std::memory_order_relaxed;
using std::size_t;
using uci::printing::info_string;

void Engine::new_game(const bool firstCall)
{
    if (not firstCall) {
        searcher.context.clear_transposition_table();
        return;
    }

    // we use delayed initialization for these callbacks instead of
    // initializing them in the constructor to avoid referencing the
    // `this` pointer in the constructor
    if (prettyPrinting.load()) {
        searcher.context.callbacks = search::Callbacks::make_pretty_printer();
    } else {
        searcher.context.callbacks = search::Callbacks::make_uci_printer(
            [this] noexcept { // cppcheck-suppress syntaxError
                return debugMode.load(memory_order_relaxed);
            });
    }
}

void Engine::set_pretty_printing(const bool shouldPrettyPrint)
{
    // check if the requested printing mode was already active
    if (prettyPrinting.exchange(shouldPrettyPrint, memory_order_relaxed) == shouldPrettyPrint)
        return;

    wait();

    if (shouldPrettyPrint) {
        searcher.context.callbacks = search::Callbacks::make_pretty_printer();
    } else {
        searcher.context.callbacks = search::Callbacks::make_uci_printer(
            [this] noexcept { return debugMode.load(memory_order_relaxed); });
    }
}

void Engine::go(const uci::GoCommandOptions& opts)
{
    searcher.start(
        opts,
        std::chrono::milliseconds { moveOverhead.get_value() });
}

// this function implements non-standard UCI commands that we support
void Engine::handle_custom_command(
    const string_view command, const string_view opts)
{
    if (const auto it = std::ranges::find(customCommands, command, &CustomCommand::name);
        it != customCommands.end()) {
        it->action(opts);
        return;
    }

    info_string(std::format("Unknown UCI command: '{}'", command));
    info_string("Type help for a list of supported commands");
}

void Engine::start_file_logger(const string_view path)
{
    if (path.empty()) {
        info_string("No path provided for file logger, not starting.");
        return;
    }

    [[maybe_unused]] const auto result
        = chess::util::start_file_logger(std::filesystem::path { path })
              .transform_error(info_string);
}

void Engine::make_null_move()
{
    wait();

    searcher.context.options.position.make_null_move();
}

void Engine::color_flip()
{
    wait();

    searcher.context.options.position.flip();
}

} // namespace ben_bot
