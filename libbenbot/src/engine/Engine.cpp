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
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <filesystem>
#include <format>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/Options.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Files.hpp>
#include <libutil/Logger.hpp>
#include <libutil/Variant.hpp>
#include <nlohmann/json.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>

namespace ben_bot {

using std::filesystem::path;
using std::size_t;
using uci::printing::info_string;

void Engine::new_game(const bool firstCall)
{
    if (firstCall) {
        // we use delayed initialization for the callbacks instead of
        // initializing them in the constructor to avoid referencing
        // the `this` pointer in the constructor
        init_search_callbacks();
    } else {
        searcher.context.clear_transposition_table();
    }
}

void Engine::init_search_callbacks()
{
    abort_search();
    wait();

    searcher.context.set_callbacks(
        create_search_callbacks());
}

auto Engine::create_search_callbacks() -> search::Callbacks
{
    if (prettyPrintMode.get_value()) {
        return search::Callbacks::make_pretty_printer(
            [this](const Move move) { return pretty_print_move(move); });
    }

    return search::Callbacks::make_uci_printer(
        [this]() noexcept { return is_debug_mode(); });
}

void Engine::go_internal(search::Options opts)
{
    opts.moveOverhead = std::chrono::milliseconds { moveOverhead.get_value() };

    searcher.context.set_options(opts);

    searcher.start();
}

void Engine::go(const search::Options& opts)
{
    searcher.context.set_pondering(
        opt_Ponder.get_value());

    go_internal(opts);
}

void Engine::go(const uci::GoCommandOptions& opts)
{
    searcher.context.set_pondering(
        opts.ponderMode and opt_Ponder.get_value());

    go_internal(
        search::Options::from_libchess(
            opts,
            searcher.context.get_position().is_white_to_move()));
}

void Engine::start_file_logger(const string_view arg)
{
    if (arg.empty()) {
        info_string("No path provided for file logger, not starting.");
        return;
    }

    [[maybe_unused]] const auto result
        = util::start_file_logger(path { arg })
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

auto Engine::state_to_string() const -> std::string
{
    json optionsData;

    auto save_option = [&optionsData](const uci::Option& opt) {
        if (not opt.has_value())
            return;

        std::visit(
            util::Visitor {
                [&optionsData, name = opt.get_name()](const auto value) {
                    optionsData[name] = value;
                } },
            opt.get_value_variant());
    };

    for (const auto* opt : options)
        save_option(*opt);

    for (const auto* opt : standardUCIOptions)
        save_option(*opt);

    json data;

    data[TAG_OPTIONS] = optionsData;
    data[TAG_DEBUG]   = is_debug_mode();

    return data.dump();
}

void Engine::write_config_file(const string_view arg) const
{
    if (arg.empty()) {
        info_string("No filepath provided for writeconfig");
        return;
    }

    const auto filePath = absolute(path { arg });

    [[maybe_unused]] const auto result
        = util::files::overwrite(
            filePath, state_to_string())
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

void Engine::restore_state_from_string(const string_view state)
{
    const auto data = json::parse(state);

    set_debug_mode(
        data.at(TAG_DEBUG).get<bool>());

    std::ranges::for_each(
        std::views::join(std::array { get_custom_uci_options(), get_standard_uci_options() }),
        [optionsData = data.at(TAG_OPTIONS)](uci::Option* opt) {
            if (not opt->has_value())
                return;

            const auto& optValue = optionsData.at(opt->get_name());

            if (auto* boolOpt = dynamic_cast<uci::BoolOption*>(opt)) {
                boolOpt->set_value(optValue.get<bool>());
                return;
            }

            if (auto* intOpt = dynamic_cast<uci::IntOption*>(opt)) {
                intOpt->set_value(optValue.get<int>());
                return;
            }

            if (auto* comboOpt = dynamic_cast<uci::ComboOption*>(opt)) {
                comboOpt->set_value(optValue.get<std::string_view>());
                return;
            }

            if (auto* stringOpt = dynamic_cast<uci::StringOption*>(opt))
                stringOpt->set_value(optValue.get<std::string_view>());
        });
}

void Engine::read_config_file(const path& file)
{
    const auto filePath = absolute(file);

    [[maybe_unused]] const auto result
        = util::files::load(filePath)
              .transform([this, &filePath](const string_view fileContent) {
                  restore_state_from_string(fileContent);

                  info_string(std::format(
                      "Read configuration from file: {}", filePath.string()));

                  return std::monostate { };
              })
              .transform_error(info_string);
}

} // namespace ben_bot
