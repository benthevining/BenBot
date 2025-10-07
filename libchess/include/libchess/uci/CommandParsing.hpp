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

/** @defgroup uci UCI
    Utilities for working with the Universal Chess Interface.

    @ingroup libchess
 */

/** @file
    This file provides utilities for parsing command-line UCI commands.
    @ingroup uci
 */

#pragma once

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <expected>
#include <libchess/moves/MoveGen.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace chess::game {
struct Position;
} // namespace chess::game

/** This namespace contains utilities for working with UCI.
    @ingroup uci
 */
namespace chess::uci {

using game::Position;
using std::chrono::milliseconds;
using std::optional;
using std::size_t;
using std::string_view;

/** Parses the options following a UCI "position" command into a Position object.
    The ``options`` should not include the "position" token itself.

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup uci
 */
[[nodiscard]] auto parse_position_options(string_view options)
    -> std::expected<Position, std::string>;

/** This struct encapsulates the options to a UCI "register" command.

    @ingroup uci
    @see parse_register_options()
 */
struct RegisterNowOptions final {
    /** The customer's name. */
    std::string name;

    /** The customer's registration code. */
    std::string code;
};

/** A simple convenience typedef.
    @ingroup uci
 */
using RegisterOptions = optional<RegisterNowOptions>;

/** Parses the options following a UCI "register" command.
    The ``options`` should not include the "register" token itself.
    If this returns ``nullopt``, then the user sent a ``register later`` command.

    @ingroup uci
    @relates RegisterOptions
 */
[[nodiscard]] auto parse_register_options(string_view options) -> RegisterOptions;

/** This struct encapsulates the options given to a UCI "go" command.

    @ingroup uci
    @see parse_go_options()
 */
struct GoCommandOptions final {
    /** The search should be restricted to only these moves.
        Empty if the "moves" suboption was not specified.
     */
    moves::MoveList moves;

    /** True if this "go" command is in ponder mode. */
    bool ponderMode { false };

    /** If true, search until the "stop" command. */
    bool infinite { false };

    /** Number of milliseconds that White has left. */
    optional<milliseconds> whiteTimeLeft;

    /** Number of milliseconds that Black has left. */
    optional<milliseconds> blackTimeLeft;

    /** Increment for White, in milliseconds. */
    optional<milliseconds> whiteInc;

    /** Increment for Black, in milliseconds. */
    optional<milliseconds> blackInc;

    /** Search for exactly this number of milliseconds. */
    optional<milliseconds> searchTime;

    /** Number of moves to the next time control. */
    optional<size_t> movesToGo;

    /** Restrict the search to this many plies only. */
    optional<size_t> depth;

    /** Search this many nodes only. */
    optional<size_t> nodes;

    /** Search for a mate in this many moves. */
    optional<size_t> mateIn;
};

/** Parses the options following a UCI "go" command.
    The ``options`` should not include the "go" token itself.

    @ingroup uci
    @relates GoCommandOptions
 */
[[nodiscard]] auto parse_go_options(
    string_view options, const Position& currentPosition)
    -> GoCommandOptions;

} // namespace chess::uci
