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
    This file defines the UCI @cite Meyer-Kahlen_2006 engine base class.
    @ingroup uci
 */

#pragma once

#include <array>
#include <functional>
#include <libchess/uci/CommandParsing.hpp>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::uci {

using game::Position;
using std::string_view;

struct Option;

/** A UCI command that the engine can respond to.
    @ingroup uci
 */
struct EngineCommand final {
    /** Function type that is invoked when this command is executed. */
    using Callback = std::function<void(string_view)>;

    /** The name of the command.
        This is the token the user should type in the CLI to execute the command.
     */
    string_view name;

    /** Function object that will be called when the command is executed.
        This callback will receive the rest of the command line as its argument.
     */
    Callback action;

    /** Brief description of this command. This will be shown in the engine's help output. */
    string_view description;

    /** A brief string to provide some documentation for the command's arguments.
        This will be shown in the engine's help output.
        For example, if the command expects a single filepath argument, this help string
        might be ``<path>``.
     */
    string_view argsHelp;

    /** Wraps a callback taking no arguments into a ``Callback`` for a command. */
    [[nodiscard]] static auto void_cb(std::function<void()>&& func) -> Callback
    {
        return [callback = std::move(func)]([[maybe_unused]] const string_view args) {
            callback();
        };
    }
};

/** A base class for UCI @cite Meyer-Kahlen_2006 chess engines.

    This class provides handling of UCI command parsing, so that
    the engine implementation can focus purely on implementing
    evaluation and search. UCI printing is provided by the functions
    in the ``uci::printing`` namespace.

    To use this class, implement a derived class, then in your
    program's ``main()`` function, create an instance of your
    derived engine class and call its ``loop()`` method.

    @ingroup uci
 */
struct EngineBase {
    EngineBase() = default;

    virtual ~EngineBase();

    EngineBase(const EngineBase&)            = default;
    EngineBase(EngineBase&&)                 = default;
    EngineBase& operator=(const EngineBase&) = default;
    EngineBase& operator=(EngineBase&&)      = default;

    /** This must return the name of the engine.
        The returned string may optionally contain the engine's current version,
        such as ``BenBot 1.2.0``.
     */
    [[nodiscard]] virtual auto get_name() const -> std::string = 0;

    /** This must return the name of the engine's author. */
    [[nodiscard]] virtual auto get_author() const -> string_view = 0;

    /** This must return the list of all options the engine supports. */
    [[nodiscard]] virtual auto get_options() -> std::span<Option*> { return { }; }

    /** This function will be called when the "isready" command is received,
        and may block while waiting for background tasks to complete. This
        function should be thread-safe.
     */
    virtual void wait() { }

    /** This function must return true if a search is currently in progress.
        This function should be thread-safe.
     */
    [[nodiscard]] virtual auto is_searching() const noexcept -> bool = 0;

    /** This function will be called when the "ucinewgame" command is received.
        This should flush any game-specific data structures such as hash tables,
        transposition table, etc. ``wait()`` will be called after this, before
        the next search begins.

        @param firstCall True if this is the first time ``new_game()`` has been
        called.
     */
    virtual void new_game([[maybe_unused]] bool firstCall) { }

    /** This function is called when the search should be exited. */
    virtual void abort_search() { }

    /** This function is called when the "ponderhit" command is received.
        This means that the engine was told to ponder on the same move the user has played.
        The engine should continue searching but switch from pondering to normal search.
     */
    virtual void ponder_hit() { }

    /** Called when a new position is received from the GUI. */
    virtual void set_position([[maybe_unused]] const Position& pos) { }

    /** Called when the "go" command is received. The engine should begin searching. After
        this function has been called, the engine should print to stdout a line of the form
        "bestmove <from><to>".
     */
    virtual void go([[maybe_unused]] const GoCommandOptions& opts) = 0;

    /** Called when the "debug" command is received. */
    virtual void set_debug([[maybe_unused]] bool shouldDebug) { }

    /** Handles a UCI command.
        Typically you will not call this directly, you'll just invoke ``loop()``, but this
        method can be used to manually invoke UCI commands if needed.

        @see loop()
     */
    void handle_command(string_view command);

    /** Runs the engine's event loop.
        This function blocks while reading from stdin. The calling thread becomes the
        engine's "main thread".

        @see handle_command()
     */
    void loop();

    /** Called when the user send a "register" command.
        The engine may simply do nothing if it does not require registration.
     */
    virtual void handle_registration([[maybe_unused]] const RegisterOptions& opts) { }

    /** Typedef for a view of a list of engine commands. */
    using CommandList = std::span<const EngineCommand>;

    /** Returns the engine's list of supported standard UCI commands. */
    [[nodiscard]] auto get_standard_uci_commands() const noexcept -> CommandList
    {
        return standardUCICommands;
    }

    /** Subclasses should overload this to return their custom UCI commands. */
    [[nodiscard]] virtual auto get_custom_uci_commands() const noexcept -> CommandList
    {
        return { };
    }

private:
    void respond_to_uci();
    void respond_to_isready();

    void handle_setpos(string_view arguments);
    void handle_setoption(string_view arguments);

    bool shouldExit { false }; // used as flag for exiting the loop() function

    bool initialized { false };

    Position position;

    std::array<EngineCommand, 11uz> standardUCICommands {
        EngineCommand {
            .name   = "uci",
            .action = EngineCommand::void_cb([this] { respond_to_uci(); }),
            .description = "Initialize UCI communication",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "isready",
            .action = EngineCommand::void_cb([this] { respond_to_isready(); }),
            .description = "Wait for engine to complete background tasks",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "ucinewgame",
            .action = EngineCommand::void_cb([this] {
                new_game(not initialized);
                initialized = true;
            }),
            .description = "Initialize a new game",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "quit",
            .action = EngineCommand::void_cb([this] {
                abort_search();
                shouldExit = true; // exit the event loop
                wait();
            }),
            .description = "Exit the engine as quickly as possible",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "stop",
            .action = EngineCommand::void_cb([this] { abort_search(); }),
            .description = "Abort the current search, if any",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "ponderhit",
            .action = EngineCommand::void_cb([this] { ponder_hit(); }),
            .description = "Indicate that the user played the ponder move",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "position",
            .action = [this](const string_view args) { handle_setpos(args); },
            .description = "Set the position on the engine's internal board",
            .argsHelp    = "[startpos|fen <fen>] [moves <move...>]" },
        EngineCommand {
            .name   = "go",
            .action = [this](const string_view args) { go(parse_go_options(args, position)); },
            .description = "Start a search",
            .argsHelp    = { } },
        EngineCommand {
            .name   = "setoption",
            .action = [this](const string_view args) { handle_setoption(args); },
            .description = "Set UCI option values",
            .argsHelp    = "[name <name>] [value <value>]" },
        EngineCommand {
            .name   = "debug",
            .action = [this](const string_view args) { set_debug(args == "on"); },
            .description = "Enable/disable engine debug mode",
            .argsHelp    = "[on|off]" },
        EngineCommand {
            .name   = "register",
            .action = [this](const string_view args) {
                handle_registration(parse_register_options(args));
            },
            .description = "Handle license registration",
            .argsHelp    = { } }
    };
};

} // namespace chess::uci
