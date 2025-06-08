/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file contains functions for creating default UCI options.
    @ingroup uci
 */

#pragma once

#include <libchess/uci/Options.hpp>

/** This namespace contains functions for creating Option objects that
    conform to the default options specified in the UCI spec.

    @ingroup uci
 */
namespace chess::uci::default_options {

/// @ingroup uci
/// @{

/** Creates the UCI "OwnBook" option.
    If this option is supported, this means that the engine has its own book
    which is accessed by the engine itself. If this is set, the engine takes
    care of the opening book and the GUI will never execute a move out of its
    book for the engine. If this is set to false by the GUI, the engine should
    not access its own book.
 */
[[nodiscard]] inline BoolOption own_book()
{
    return { "OwnBook", true };
}

/** Creates the UCI "Ponder" option.
    If this option is supported, this means that the engine is able to ponder.
    The GUI will send this whenever pondering is possible or not. Note: The engine
    should not start pondering on its own if this is enabled, this option is only
    needed because the engine might change its time management algorithm when
    pondering is allowed.
 */
[[nodiscard]] inline BoolOption ponder()
{
    return { "Ponder", true };
}

/** Creates the "UCI_ShowCurrLine" option.
    If this option is supported, this means that the engine can show the current
    line it is calculating (i.e. by printing "info currline" output).
 */
[[nodiscard]] inline BoolOption show_curr_line()
{
    return { "UCI_ShowCurrLine", false };
}

/** Creates the "UCI_ShowRefutations" option.
    If this option is supported, this means that the engine can show a move and
    its refutation in a line (i.e. by printing "info refutations" output).
 */
[[nodiscard]] inline BoolOption show_refutations()
{
    return { "UCI_ShowRefutations", false };
}

/** Creates the "UCI_AnalyseMode" option.
    If this option is supported, this means that the engine wants to behave differently
    when analyzing or playing a game. For example, when playing it can use some kind of
    learning. This is set to false if the engine is playing a game, otherwise it is true.
 */
[[nodiscard]] inline BoolOption analyze_mode()
{
    return { "UCI_AnalyseMode", false };
}

/// @}

} // namespace chess::uci::default_options
