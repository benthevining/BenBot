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

/** @file
    This file defines the top-level engine class used by the BenBot executable.
    @ingroup benbot
 */

#pragma once

#include <array>
#include <atomic>
#include <ben-bot/CustomCommand.hpp>
#include <functional>
#include <libbenbot/search/Search.hpp>
#include <libbenbot/search/Thread.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/uci/CommandParsing.hpp> // IWYU pragma: keep - for uci::GoCommandOptions
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Options.hpp>
#include <span>
#include <string_view>

namespace ben_bot {

using std::string_view;

namespace uci = chess::uci;

/** The ``ben-bot`` UCI engine class.
    @ingroup benbot
 */
class Engine final : public uci::EngineBase {
public:
    Engine();

    /** Prints the engine's logo and version to ``stdout``. */
    void print_logo_and_version() const;

private:
    [[nodiscard]] string_view get_name() const override;
    [[nodiscard]] string_view get_author() const override { return "Ben Vining"; }

    void new_game(bool firstCall) override;

    void set_position(const Position& pos) override { searcher.set_position(pos); }

    void go(uci::GoCommandOptions&& opts) override;

    void abort_search() override { searcher.context.abort(); }

    void wait() override { searcher.context.wait(); }

    bool is_searching() const noexcept override { return searcher.context.in_progress(); }

    void set_debug(const bool shouldDebug) override { debugMode.store(shouldDebug); }

    [[nodiscard]] std::span<uci::Option*> get_options() override { return options; }

    void handle_custom_command(string_view command, string_view opts) override;

    void run_perft(string_view arguments) const;

    static void run_bench(string_view arguments);

    void make_null_move();

    void print_help(string_view args) const;
    void print_options() const;
    void print_current_position(string_view arguments) const;

    static void print_compiler_info();

    std::atomic_bool debugMode { false };

    search::Thread searcher;

    uci::Action clearTT {
        "Clear Hash",
        [this] { searcher.context.clear_transposition_table(); },
        "Press to clear the transposition table"
    };

    std::array<uci::Option*, 1uz> options {
        &clearTT
    };

    // clang-format off
    std::array<CustomCommand, 7uz> customCommands {
        CustomCommand {
            .name = "showpos",
            .action = [this](const string_view args){ print_current_position(args); },
            .description = "Prints the current position",
            .argsHelp = "[utf8]"
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
            .name = "perft",
            .action = [this](const string_view args) { run_perft(args); },
            .description = "Computes perft of the current position to the given depth",
            .argsHelp = "<N>"
        },
        CustomCommand {
            .name = "bench",
            .action = [](const string_view args){ run_bench(args); },
            .description = "Runs a search and reports total nodes and NPS",
            .argsHelp = "[<depth>] [<epdPath>]"
        },
        CustomCommand {
            .name = "compiler",
            .action = CustomCommand::void_cb([]{ print_compiler_info(); }),
            .description = "Print compiler info"
        },
        CustomCommand {
            .name = "help",
            .action = [this] (const string_view args){ print_help(args); },
            .description = "Display this text",
            .argsHelp = "[--no-logo]"
        }
    };
    // clang-format on
};

} // namespace ben_bot
