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

#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <libbenbot/search/Search.hpp>
#include <libbenbot/search/Thread.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/uci/CommandParsing.hpp> // IWYU pragma: keep - for uci::GoCommandOptions
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Options.hpp>
#include <span>
#include <string_view>
#include <utility>

namespace ben_bot {

using std::string_view;

namespace uci    = chess::uci;
namespace search = chess::search;

class Engine final : public uci::EngineBase {
public:
    void print_logo_and_version() const;

private:
    [[nodiscard]] string_view get_name() const override { return "BenBot"; }
    [[nodiscard]] string_view get_author() const override { return "Ben Vining"; }

    void new_game(bool firstCall) override;

    void set_position(const chess::game::Position& pos) override { searcher.set_position(pos); }

    void go(uci::GoCommandOptions&& opts) override { searcher.start(std::move(opts)); }

    void abort_search() override { searcher.context.abort(); }

    void wait() override { searcher.context.wait(); }

    [[nodiscard]] std::span<uci::Option*> get_options() override { return options; }

    void handle_custom_command(string_view command, string_view options) override;

    void load_book_file(const std::filesystem::path& file);

    void print_help() const;
    void print_options() const;
    void print_current_position() const;

    struct CustomCommand final {
        using Callback = std::function<void(string_view)>;

        string_view name;

        Callback action;

        string_view description;

        string_view argsHelp;

        [[nodiscard]] static Callback void_cb(std::function<void()>&& func)
        {
            return [callback = std::move(func)]([[maybe_unused]] const string_view args) {
                callback();
            };
        }
    };

    search::Thread searcher { search::Callbacks::make_uci_handler() };

    uci::Action clearTT {
        "Clear Hash",
        [this] { searcher.context.reset(); },
        "Press to clear the transposition table"
    };

    std::array<uci::Option*, 2uz> options {
        &searcher.context.openingBook.enabled,
        &clearTT
    };

    // clang-format off
    std::array<CustomCommand, 4uz> customCommands {
        CustomCommand {
            .name   = "loadbook",
            .action = [this](const string_view args) {
                load_book_file(std::filesystem::path { args });
            },
            .description = "Reads the given JSON file into the engine's openings database. See book.json in the ben-bot source code for an example of the format.",
            .argsHelp = "<path>"
        },
        CustomCommand {
            .name = "showpos",
            .action = CustomCommand::void_cb([this] { print_current_position(); }),
            .description = "Prints the current position"
        },
        CustomCommand {
            .name = "options",
            .action = CustomCommand::void_cb([this] { print_options(); }),
            .description = "Dump current UCI option values"
        },
        CustomCommand {
            .name = "help",
            .action = CustomCommand::void_cb([this] { print_help(); }),
            .description = "Display this text"
        }
    };
    // clang-format on
};

} // namespace ben_bot
