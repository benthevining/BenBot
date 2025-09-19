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
#include <libbenbot/search/Thread.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/uci/DefaultOptions.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Options.hpp>
#include <span>
#include <string>
#include <string_view>

namespace chess::uci {
struct GoCommandOptions;
} // namespace chess::uci

namespace ben_bot {

using chess::game::Position;
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
    [[nodiscard]] std::string get_name() const override;
    [[nodiscard]] string_view get_author() const override { return "Ben Vining"; }

    void new_game(bool firstCall) override;

    void set_position(const Position& pos) override { searcher.set_position(pos); }

    void go(uci::GoCommandOptions&& opts) override;

    void abort_search() override { searcher.context.abort(); }

    void ponder_hit() override { searcher.context.ponder_hit(); }

    void wait() override { searcher.context.wait(); }

    bool is_searching() const noexcept override { return searcher.context.in_progress(); }

    void set_debug(const bool shouldDebug) override { debugMode.store(shouldDebug); }

    [[nodiscard]] std::span<uci::Option*> get_options() override { return options; }

    void handle_custom_command(string_view command, string_view opts) override;

    void run_perft(string_view arguments) const;

    static void run_bench(string_view arguments);

    void make_null_move();
    void color_flip();

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

    // NB. The engine doesn't start pondering on its own without explicitly being told to
    // via another go command; this option is needed to inform the GUI that the engine
    // supports pondering, and also gives the engine the opportunity to adjust its time
    // management algorithm when pondering is enabled.
    uci::BoolOption ponder { uci::default_options::ponder() };

    std::array<uci::Option*, 2uz> options { &clearTT, &ponder };

    // clang-format off
    std::array<CustomCommand, 8uz> customCommands {
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
            .name = "flip",
            .action = CustomCommand::void_cb([this]{ color_flip(); }),
            .description = "Color-flip the current position"
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
