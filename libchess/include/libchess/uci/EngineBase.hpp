/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the UCI engine base class.
    @ingroup uci
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/Options.hpp>
#include <span>
#include <string_view>

namespace chess::uci {

using game::Position;

/** A base class for UCI chess engines.
    This class provides handling of UCI command printing and
    parsing, so that the engine implementation can focus purely
    on implementing evaluation and search.

    @ingroup uci

    @todo "info" command to GUI
 */
struct EngineBase {
    EngineBase() = default;

    virtual ~EngineBase() = default;

    EngineBase(const EngineBase&)            = default;
    EngineBase(EngineBase&&)                 = default;
    EngineBase& operator=(const EngineBase&) = default;
    EngineBase& operator=(EngineBase&&)      = default;

    /** This must return the name of the engine. */
    [[nodiscard]] virtual std::string_view get_name() const = 0;

    /** This must return the name of the engine's author. */
    [[nodiscard]] virtual std::string_view get_author() const = 0;

    /** This must return the list of all options the engine supports. */
    [[nodiscard]] virtual std::span<Option*> get_options() { return {}; }

    /** This function will be called when the "isready" command is received,
        and may block while waiting for background tasks to complete.
     */
    virtual void wait() { }

    /** This function will be called when the "ucinewgame" command is received.
        This should flush any game-specific data structures such as hash tables,
        transposition table, etc.
     */
    virtual void new_game() { }

    /** This function is called when the search should be exited as quickly as possible. */
    virtual void abort_search() { }

    /** This function is called when the search should be stopped, but with less urgency
        than ``abort_search()``.
     */
    virtual void stop_search() { }

    /** This function is called when the "ponderhit" command is received.
        This means that the engine was told to ponder on the same move the user has played.
        The engine should continue searching but switch from pondering to normal search.
     */
    virtual void ponder_hit() { }

    /** Called when a new position is received from the GUI. */
    virtual void set_position([[maybe_unused]] const Position& pos) { }

    /** Called when the "go" command is received. The engine should begin searching. */
    virtual void go([[maybe_unused]] const GoCommandOptions& opts) { }

    /** Called after any option has changed.
        The main thread will first call ``parse()`` for each option, and then call this function.
     */
    virtual void options_changed() { }

    /** Called when the "debug" command is received. */
    virtual void set_debug([[maybe_unused]] bool shouldDebug) { }

    /** Handles a UCI command read from the command line.
        Programs will typically not need to call this directly.
     */
    void handle_command(std::string_view command);

    /** Runs the engine's event loop.
        This function blocks while reading from stdin. The calling thread becomes the
        engine's "main thread".
     */
    void loop();

private:
    bool shouldExit { false }; // used as flag for exiting the loop() function

    Position position;
};

} // namespace chess::uci
