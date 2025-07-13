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
#include <atomic>
#include <filesystem>
#include <functional>
#include <libbenbot/search/Search.hpp>
#include <libbenbot/search/Thread.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
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

class Engine final : public uci::EngineBase {
public:
    void print_logo_and_version() const;

private:
    [[nodiscard]] string_view get_name() const override { return "BenBot"; }
    [[nodiscard]] string_view get_author() const override { return "Ben Vining"; }

    void new_game(bool firstCall) override;

    void set_position(const Position& pos) override { searcher.set_position(pos); }

    void go(uci::GoCommandOptions&& opts) override;

    void ponder_hit() override;

    void abort_search() override { searcher.context.abort(); }

    void wait() override { searcher.context.wait(); }

    void set_debug(const bool shouldDebug) override { debugMode.store(shouldDebug); }

    [[nodiscard]] std::span<uci::Option*> get_options() override { return options; }

    void handle_custom_command(string_view command, string_view options) override;

    void load_book_file(string_view arguments);

    void make_null_move();

    void print_help() const;
    void print_options() const;
    void print_current_position() const;

    static void print_compiler_info();

    using Result = search::Callbacks::Result;

    template <bool PrintBestMove>
    void print_uci_info(const Result& res) const;

    void print_book_hit() const;

    std::optional<Move> ponderMove; // TODO: atomic?

    std::atomic_bool debugMode { false };

    search::Thread searcher { search::Callbacks {
        .onSearchComplete = [this](const Result& res) {
            ponderMove = res.bestResponse;
            print_uci_info<true>(res); },
        .onIteration      = [this](const Result& res) { print_uci_info<false>(res); },
        .onOpeningBookHit = [this]([[maybe_unused]] const Move& move) { print_book_hit(); } } };

    uci::Action clearTT {
        "Clear Hash",
        [this] { searcher.context.clear_transposition_table(); },
        "Press to clear the transposition table"
    };

    uci::BoolOption ponderOpt {
        uci::default_options::ponder()
    };

    std::array<uci::Option*, 3uz> options {
        &searcher.context.openingBook.enabled,
        &ponderOpt, &clearTT
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
