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
#include <libchess/uci/CommandParsing.hpp> // IWYU pragma: keep - for GoCommandOptions
#include <libchess/uci/Options.hpp>
#include <span>
#include <string_view>

namespace chess::uci {

using game::Position;
using std::string_view;

/** A base class for UCI chess engines.
    This class provides handling of UCI command printing and
    parsing, so that the engine implementation can focus purely
    on implementing evaluation and search.

    To use one of these, implement a derived class, then in your
    program's ``main()`` function, create an instance of your
    derived engine class and call its ``loop()`` method.

    @ingroup uci
 */
struct EngineBase {
    EngineBase() = default;

    virtual ~EngineBase() = default;

    EngineBase(const EngineBase&)            = default;
    EngineBase(EngineBase&&)                 = default;
    EngineBase& operator=(const EngineBase&) = default;
    EngineBase& operator=(EngineBase&&)      = default;

    /** This must return the name of the engine. */
    [[nodiscard]] virtual string_view get_name() const = 0;

    /** This must return the name of the engine's author. */
    [[nodiscard]] virtual string_view get_author() const = 0;

    /** This must return the list of all options the engine supports. */
    [[nodiscard]] virtual std::span<Option*> get_options() { return {}; }

    /** This function will be called when the "isready" command is received,
        and may block while waiting for background tasks to complete.
     */
    virtual void wait() { }

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
    virtual void go([[maybe_unused]] GoCommandOptions&& opts) { }

    /** Called after any option has changed.
        The main thread will first call ``parse()`` for each option, and then call this function
        if any options have been changed. This function will not be called if an unrecognized
        option name was specified to ``setoption``.
     */
    virtual void options_changed() { }

    /** Called when the "debug" command is received. */
    virtual void set_debug([[maybe_unused]] bool shouldDebug) { }

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
     */
    void loop();

private:
    void handle_command(string_view command);

    void respond_to_uci();

    void handle_setoption(string_view arguments);

    bool shouldExit { false }; // used as flag for exiting the loop() function

    bool initialized { false };

    Position position;
};

} // namespace chess::uci
