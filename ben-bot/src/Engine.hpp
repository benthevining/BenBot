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

using std::filesystem::path;
using std::string_view;

namespace uci = chess::uci;

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

    void load_book_file(string_view arguments);

    void make_null_move();

    void print_help() const;
    void print_options() const;
    void print_current_position() const;

    static void print_compiler_info();

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
        [this] { searcher.context.clear_transposition_table(); },
        "Press to clear the transposition table"
    };

    std::array<uci::Option*, 2uz> options {
        &searcher.context.openingBook.enabled,
        &clearTT
    };

    // clang-format off
    std::array<CustomCommand, 6uz> customCommands {
        CustomCommand {
            .name   = "loadbook",
            .action = [this](const string_view args) {
                load_book_file(args);
            },
            .description = "Reads the given PGN file into the engine's openings database",
            .argsHelp = "<path> [novars]"
        },
        CustomCommand {
            .name = "showpos",
            .action = CustomCommand::void_cb([this] { print_current_position(); }),
            .description = "Prints the current position"
        },
        CustomCommand {
            .name = "makenull",
            .action = CustomCommand::void_cb([this]{ make_null_move(); }),
            .description = "Play a null move on the internal board"
        },
        CustomCommand {
            .name = "options",
            .action = CustomCommand::void_cb([this] { print_options(); }),
            .description = "Dump current UCI option values"
        },
        CustomCommand {
            .name = "compiler",
            .action = CustomCommand::void_cb([]{ print_compiler_info(); }),
            .description = "Print compiler info"
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
