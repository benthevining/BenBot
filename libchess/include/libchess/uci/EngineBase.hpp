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
    This file defines the UCI engine base class.
    @ingroup uci
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <libchess/uci/CommandParsing.hpp> // IWYU pragma: keep
#include <libchess/uci/Options.hpp>
#include <span>
#include <string>
#include <string_view>

namespace chess::uci {

using game::Position;
using std::string_view;

/** A base class for UCI chess engines.

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
    [[nodiscard]] virtual auto get_options() -> std::span<Option*> { return {}; }

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

    /** Any command input string not recognized as a standard UCI command will invoke this function.
        Engines can implement custom commands by overriding this function. The "command" argument
        will be the first word of the input command line.
     */
    virtual void handle_custom_command(
        [[maybe_unused]] string_view command,
        [[maybe_unused]] string_view options) { }

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

private:
    void respond_to_uci();

    void handle_setpos(string_view arguments);
    void handle_setoption(string_view arguments);

    bool shouldExit { false }; // used as flag for exiting the loop() function

    bool initialized { false };

    Position position;
};

} // namespace chess::uci
