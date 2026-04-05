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
    @ingroup libbenbot
 */

#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <libbenbot/search/Thread.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Options.hpp>
#include <span>
#include <string>
#include <string_view>

namespace chess::uci {
struct GoCommandOptions;
} // namespace chess::uci

namespace ben_bot {

namespace search {
    struct Options;
} // namespace search

namespace uci = chess::uci;

using chess::game::Position;
using chess::moves::Move;
using chess::notation::MoveFormat;
using std::string_view;
using uci::EngineCommand;

/** The ``ben-bot`` UCI engine class.
    @ingroup libbenbot
 */
class [[nodiscard]] Engine : public uci::EngineBase {
public:
    Engine() = default;

    ~Engine() override = default;

    Engine(const Engine&)            = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&)            = delete;
    Engine& operator=(Engine&&) = delete;

    /** Loads the engine's state from a configuration file at the given path. */
    void read_config_file(const std::filesystem::path& file);

    /** Saves the engine's state to a string. */
    [[nodiscard]] auto state_to_string() const -> std::string;

    /** Restores the engine's state from a stringified version. */
    void restore_state_from_string(string_view state);

    /** Sets the options to be used for the next search. */
    void set_search_options(const search::Options& opts) { searcher.context.set_options(opts); }

    /** Returns the engine's current search parameters, set by the last go command. */
    [[nodiscard]] auto get_search_options() const noexcept -> const search::Options&
    {
        return searcher.context.get_options();
    }

    /** Returns the notation format being used for pretty printing. */
    [[nodiscard]] auto get_move_format() const -> MoveFormat { return moveFormat.get_enum_value(); }

    /** Starts a search with the specified options. */
    void go(const search::Options& opts);

protected:
    /** This function is used to initialize the search context's result callbacks.
        The default implementation returns either a console pretty printer or a
        UCI printer depending on the value of the pretty printing parameter, but
        other engine use cases can override this to return other sets of callbacks.
     */
    [[nodiscard]] virtual auto create_search_callbacks() -> search::Callbacks;

private:
    [[nodiscard]] auto get_name() const -> std::string override;
    [[nodiscard]] auto get_author() const -> string_view override { return "Ben Vining"; }

    void new_game(bool firstCall) override;

    void position_changed(const Position& pos) override { searcher.context.set_position(pos); }

    void go(const uci::GoCommandOptions& opts) override;

    void go_internal(search::Options opts);

    void abort_search() override { searcher.context.abort(); }

    void ponder_hit() override { searcher.context.ponder_hit(); }

    void wait() override { searcher.context.wait(); }

    auto is_searching() const noexcept -> bool override { return searcher.context.in_progress(); }

    [[nodiscard]] auto get_custom_uci_options() noexcept -> OptionList override
    {
        return options;
    }

    [[nodiscard]] auto get_custom_uci_commands() const noexcept -> CommandList override
    {
        return customCommands;
    }

    void run_perft(string_view arguments) const;

    void run_bench(string_view arguments) const;

    void make_null_move();
    void color_flip();

    void print_logo_and_version() const;
    void print_help(string_view args) const;
    void print_options();
    void print_current_position(string_view arguments) const;

    void init_search_callbacks();

    [[nodiscard]] auto pretty_print_move(Move move) const -> std::string;

    static void print_compiler_info();

    static void start_file_logger(string_view arg);

    void write_config_file(string_view arg) const;
    void read_config_file(string_view arg);

    void resize_transposition_table(const size_t sizeMB) override
    {
        searcher.context.resize_transposition_table(sizeMB);
    }

    void set_ponder(const bool shouldPonder) override
    {
        // the ponder flag is only ever turned on via the go options,
        // but it can be turned off by disabling this UCI option
        if (not shouldPonder)
            searcher.context.set_pondering(false);
    }

    search::Thread searcher;

    /* ----- UCI options ----- */

    uci::Action clearTT {
        "Clear Hash",
        [this] { searcher.context.clear_transposition_table(); },
        "Press to clear the transposition table."
    };

    uci::IntOption threads {
        "Threads", 1, 1, 1,
        "Number of searcher threads (currently a dummy)."
    };

    uci::IntOption moveOverhead {
        "Move Overhead", 0, 5000, 10,
        "Extra overhead time subtracted from search time (in ms). Increase this if engine loses on time."
    };

    uci::StringOption logFile {
        "Debug Log File", "<empty>",
        "If not empty, engine I/O will be mirrored to this file.",
        start_file_logger
    };

    uci::BoolOption prettyPrintMode {
        "Pretty Print",
        false,
        "When on, search output is pretty-printed instead of printed in UCI format.",
        [this]([[maybe_unused]] const bool usePretty) { init_search_callbacks(); }
    };

    uci::EnumOption<MoveFormat> moveFormat {
        "Move Format",
        MoveFormat::Algebraic,
        "Notation format used to display moves in pretty printing mode."
    };

    uci::BoolOption algFormatUTF8PieceType {
        "Algebraic Format UTF8",
        false,
        "When on and printing algebraic move format, uses UTF8 glyphs for piece types."
    };

    uci::BoolOption sanitizePositions {
        "Sanitize Positions",
        false,
        "When on, the engine checks if the position is legal before setting it.",
        [this](const bool sanitize) { set_sanitize_positions(sanitize); }
    };

    std::array<uci::Option*, 8uz> options {
        &clearTT, &threads, &moveOverhead, &logFile, &prettyPrintMode, &moveFormat, &algFormatUTF8PieceType, &sanitizePositions
    };

    std::array<EngineCommand, 10uz> customCommands {
        EngineCommand {
            .name   = "showpos",
            .action = [this](const string_view args) { print_current_position(args); },
            .description = "Prints the current position",
            .argsHelp    = "[utf8]" },
        EngineCommand {
            .name   = "makenull",
            .action = EngineCommand::void_cb([this] { make_null_move(); }),
            .description = "Play a null move on the internal board",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "flip",
            .action = EngineCommand::void_cb([this] { color_flip(); }),
            .description = "Color-flip the current position",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "options",
            .action = EngineCommand::void_cb([this] { print_options(); }),
            .description = "Dump current UCI option values",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "perft",
            .action = [this](const string_view args) { run_perft(args); },
            .description = "Computes perft of the current position to the given depth",
            .argsHelp    = "<N> [json]" },
        EngineCommand {
            .name   = "bench",
            .action = [this](const string_view args) { run_bench(args); },
            .description = "Runs a search and reports total nodes and NPS",
            .argsHelp    = "[<depth>] [<epdPath>]" },
        EngineCommand {
            .name   = "compiler",
            .action = EngineCommand::void_cb([] { print_compiler_info(); }),
            .description = "Print compiler info",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "writeconfig",
            .action = [this](const string_view args) { write_config_file(args); },
            .description = "Writes the engine's current state to a configuration file at the given path",
            .argsHelp    = "<path>" },
        EngineCommand {
            .name   = "readconfig",
            .action = [this](const string_view args) { read_config_file(args); },
            .description = "Loads engine state from a configuration file at the given path",
            .argsHelp    = "<path>" },
        EngineCommand {
            .name   = "help",
            .action = [this](const string_view args) { print_help(args); },
            .description = "Display this text",
            .argsHelp    = "[--no-logo]" }
    };
};

} // namespace ben_bot
