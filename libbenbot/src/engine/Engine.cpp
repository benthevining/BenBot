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
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <filesystem>
#include <format>
#include <fstream>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Files.hpp>
#include <libchess/util/Logger.hpp>
#include <libchess/util/Variant.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ben_bot {

using std::memory_order_relaxed;
using std::size_t;
using uci::printing::info_string;

auto Engine::create_move_format_option() -> uci::ComboOption
{
    using chess::notation::MoveFormat;

    auto strings = magic_enum::enum_names<MoveFormat>()
                 | std::views::transform([](const std::string_view str) {
                       return std::string { str };
                   })
                 | std::ranges::to<std::vector>();

    std::string valueList;

    for (const auto& value : strings | std::views::take(strings.size() - 1uz)) {
        valueList.append(value);
        valueList.append(", ");
    }

    valueList.append(strings.back());

    return { "Move Format",
        std::move(strings),
        std::string { magic_enum::enum_name(MoveFormat::Algebraic) },
        std::format(
            "Notation format used to display moves in pretty printing mode. ({})",
            valueList) };
}

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
        searcher.context.callbacks = search::Callbacks::make_pretty_printer(
            [this](const Move move) { return pretty_print_move(move); });
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
        searcher.context.callbacks = search::Callbacks::make_pretty_printer(
            [this](const Move move) { return pretty_print_move(move); });
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

using nlohmann::json;

inline constexpr string_view TAG_OPTIONS { "uci_options" };
inline constexpr string_view TAG_DEBUG { "debug" };

void Engine::write_config_file(const string_view path) const
{
    if (path.empty()) {
        info_string("No filepath provided for writeconfig");
        return;
    }

    json optionsData;

    for (const auto* opt : options) {
        if (not opt->has_value())
            continue;

        std::visit(
            chess::util::Visitor {
                [&optionsData, name = opt->get_name()](const auto value) {
                    optionsData[name] = value;
                } },
            opt->get_value_variant());
    }

    json data;

    data[TAG_OPTIONS] = optionsData;
    data[TAG_DEBUG]   = debugMode.load(memory_order_relaxed);

    std::ofstream stream { std::filesystem::path { path } };

    stream << data.dump(2) << '\n';
}

void Engine::read_config_file(const string_view path)
{
    if (path.empty()) {
        info_string("No filepath provided for readconfig");
        return;
    }

    [[maybe_unused]] const auto result = chess::util::load_file_as_string(std::filesystem::path { path })
                                             .transform([this](const string_view fileContent) {
                                                 const auto data = json::parse(fileContent);

                                                 debugMode.store(
                                                     data.at(TAG_DEBUG).get<bool>(),
                                                     memory_order_relaxed);

                                                 const auto& optionsData = data.at(TAG_OPTIONS);

                                                 for (const auto* opt : options) {
                                                     if (not opt->has_value())
                                                         continue;

                                                     std::visit(
                                                         chess::util::Visitor {
                                                             [&optionsData, name = opt->get_name()](const auto value) {
                                                                 // TODO: set option value from optionsData.at(name)
                                                             } },
                                                         opt->get_value_variant());
                                                 }

                                                 return std::monostate { };
                                             })
                                             .transform_error(info_string);
}

} // namespace ben_bot
