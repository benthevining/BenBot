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

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>
#include <string_view>
#include <vector>

/** This namespace contains utilities for working with UCI.
    @ingroup uci
 */
namespace chess::uci {

using Milliseconds = std::chrono::milliseconds;

using game::Position;
using std::size_t;

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

    /** If true, search until the "stop" command. */
    bool infinite { false };

    /** Number of milliseconds that White has left. */
    std::optional<Milliseconds> whiteTimeLeft;

    /** Number of milliseconds that Black has left. */
    std::optional<Milliseconds> blackTimeLeft;

    /** Increment for White, in milliseconds. */
    std::optional<Milliseconds> whiteInc;

    /** Increment for Black, in milliseconds. */
    std::optional<Milliseconds> blackInc;

    /** Search for exactly this number of milliseconds. */
    std::optional<Milliseconds> searchTime;

    /** Number of moves to the next time control. */
    std::optional<size_t> movesToGo;

    /** Restrict the search to this many plies only. */
    std::optional<size_t> depth;

    /** Search this many nodes only. */
    std::optional<size_t> nodes;

    /** Search for a mate in this many moves. */
    std::optional<size_t> mateIn;
};

/** Parses the options following a UCI "go" command.
    The ``options`` should not include the "go" token itself.

    @ingroup uci
    @relates GoCommandOptions
 */
[[nodiscard]] GoCommandOptions parse_go_options(
    std::string_view options, const Position& currentPosition);

} // namespace chess::uci
