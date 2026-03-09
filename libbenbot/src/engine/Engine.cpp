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
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/Options.hpp>
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

using std::filesystem::path;
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
    if (prettyPrintMode.get_value()) {
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
    wait();

    if (shouldPrettyPrint) {
        searcher.context.callbacks = search::Callbacks::make_pretty_printer(
            [this](const Move move) { return pretty_print_move(move); });
    } else {
        searcher.context.callbacks = search::Callbacks::make_uci_printer(
            [this] noexcept { return debugMode.load(memory_order_relaxed); });
    }
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

void Engine::start_file_logger(const string_view arg)
{
    if (arg.empty()) {
        info_string("No path provided for file logger, not starting.");
        return;
    }

    [[maybe_unused]] const auto result
        = chess::util::start_file_logger(path { arg })
              .transform_error(info_string);
}

void Engine::make_null_move()
{
    searcher.context.set_position(
        after_null_move(searcher.context.get_position()));
}

void Engine::color_flip()
{
    searcher.context.set_position(
        flipped(searcher.context.get_position()));
}

using nlohmann::json;

inline constexpr string_view TAG_OPTIONS { "uci_options" };
inline constexpr string_view TAG_DEBUG { "debug" };

void Engine::write_config_file(const string_view arg) const
{
    if (arg.empty()) {
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

    const auto filePath = absolute(path { arg });

    [[maybe_unused]] const auto result
        = chess::util::overwrite_file(
            filePath, data.dump(2))
              .transform([&filePath] {
                  info_string(std::format(
                      "Wrote configuration file to: {}", filePath.string()));
              })
              .transform_error(info_string);
}

void Engine::read_config_file(const string_view arg)
{
    if (arg.empty()) {
        info_string("No filepath provided for readconfig");
        return;
    }

    read_config_file(path { arg });
}

void Engine::read_config_file(const path& file)
{
    const auto filePath = absolute(file);

    [[maybe_unused]] const auto result
        = chess::util::load_file_as_string(filePath)
              .transform([this, &filePath](const string_view fileContent) {
                  const auto data = json::parse(fileContent);

                  debugMode.store(
                      data.at(TAG_DEBUG).get<bool>(),
                      memory_order_relaxed);

                  const auto& optionsData = data.at(TAG_OPTIONS);

                  for (auto* opt : options) {
                      if (not opt->has_value())
                          continue;

                      const auto& optValue = optionsData.at(opt->get_name());

                      namespace uci = chess::uci;

                      if (auto* boolOpt = dynamic_cast<uci::BoolOption*>(opt)) {
                          boolOpt->set_value(optValue.get<bool>());
                          continue;
                      }

                      if (auto* intOpt = dynamic_cast<uci::IntOption*>(opt)) {
                          intOpt->set_value(optValue.get<int>());
                          continue;
                      }

                      if (auto* comboOpt = dynamic_cast<uci::ComboOption*>(opt)) {
                          comboOpt->set_value(optValue.get<string_view>());
                          continue;
                      }

                      if (auto* stringOpt = dynamic_cast<uci::StringOption*>(opt)) {
                          stringOpt->set_value(optValue.get<string_view>());
                      }
                  }

                  info_string(std::format(
                      "Read configuration from file: {}", filePath.string()));

                  return std::monostate { };
              })
              .transform_error(info_string);
}

} // namespace ben_bot
