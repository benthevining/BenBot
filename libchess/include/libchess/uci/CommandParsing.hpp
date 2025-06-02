/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup uci UCI
    Utilities for working with the Universal Chess Interface.
 */

/** @file
    This file provides utilities for parsing command-line UCI commands.
    @ingroup uci
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <string_view>
#include <vector>

/** This namespace contains utilities for working with UCI.
    @ingroup uci
 */
namespace chess::uci {

using game::Position;

/** Parses the options following a UCI "position" command into a Position object.
    The ``options`` should not include the "position" token itself.

    @ingroup uci
 */
[[nodiscard]] Position parse_position_options(std::string_view options);

/** This struct encapsulates the options given to a UCI "go" command.

    @ingroup uci
 */
struct GoCommandOptions final {
    /** The search should be restricted to only these moves.
        Empty if the "moves" suboption was not specified.
     */
    std::vector<moves::Move> moves;

    /** True if this "go" command is in ponder mode. */
    bool ponderMode { false };

    /** Number of milliseconds that White has left. */
    size_t whiteMsLeft { 0uz };

    /** Number of milliseconds that Black has left. */
    size_t blackMsLeft { 0uz };

    /** Increment for White, in milliseconds. */
    size_t whiteIncMs { 0uz };

    /** Increment for Black, in milliseconds. */
    size_t blackIncMs { 0uz };

    /** Number of moves to the next time control. */
    size_t movesToGo { 0uz };

    /** Restrict the search to this many plies only. */
    size_t depth { 0uz };

    /** Search this many nodes only. */
    size_t nodes { 0uz };

    /** Search for a mate in this many moves. */
    size_t mateIn { 0uz };

    /** Search for exactly this number of milliseconds. */
    size_t searchTime { 0uz };

    /** If true, search until the "stop" command. */
    bool infinite { false };
};

/** Parses the options following a UCI "go" command.
    The ``options`` should not include the "go" token itself.

    @ingroup uci
    @relates GoCommandOptions
 */
[[nodiscard]] GoCommandOptions parse_go_options(
    std::string_view options, const Position& currentPosition);

} // namespace chess::uci
