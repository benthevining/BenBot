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
#include <ben-bot/Engine.hpp>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <filesystem>
#include <format>
#include <libbenbot/search/Callbacks.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Logger.hpp>
#include <string>
#include <utility>

namespace ben_bot {

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
    searcher.context.callbacks = search::Callbacks::make_uci_printer(
        [this] { return debugMode.load(); });
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
    if (const auto it = std::ranges::find_if(
            customCommands,
            [command](const CustomCommand& cmd) { return cmd.name == command; });
        it != customCommands.end()) {
        it->action(opts);
        return;
    }

    [[unlikely]];

    info_string(std::format("Unknown UCI command: '{}'", command));
    info_string("Type help for a list of supported commands");
}

void Engine::start_file_logger(const string_view path)
{
    if (path.empty())
        return;

    [[maybe_unused]] const auto result
        = chess::util::start_file_logger(std::filesystem::path { path })
              .transform_error([](const string_view error) {
                  info_string(error);
                  return std::monostate {};
              });
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
