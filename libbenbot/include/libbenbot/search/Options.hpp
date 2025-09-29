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

/** @defgroup search Search
    Functions for searching to find the best move.

    @ingroup libbenbot
 */

/** @file
    This file defines the search options struct.
    @ingroup search
 */

#pragma once

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <limits>
#include <optional>

namespace chess::uci {
struct GoCommandOptions;
} // namespace chess::uci

/** This namespace contains functions related to searching the
    move tree for the principal variation.

    @ingroup search
 */
namespace ben_bot::search {

using std::size_t;

/** This struct encapsulates the parameters to the search algorithm.

    @ingroup search
    @see Context
 */
struct Options final {
    /** The root position to be searched. */
    chess::game::Position position;

    /** The maximum search depth (in plies). */
    size_t depth { std::numeric_limits<size_t>::max() };

    /** The maximum search time. */
    std::optional<std::chrono::milliseconds> searchTime;

    /** Search only this many nodes. Setting this value too low can
        really emphasize the effects of our move ordering algorithm.
     */
    std::optional<size_t> maxNodes;

    /** Restrict the search to only these moves.
        If this is empty, all legal moves in the position will be searched.
     */
    chess::moves::MoveList movesToSearch;

    /** If true, the search should not exit until a ``xtop`` or ``exit``
        command is received.
     */
    bool infinite { false };

    /** Search for mate in this many moves. */
    std::optional<size_t> mateIn;

    /** Updates the values in this options struct with the UCI "go" command options. */
    void update_from(const chess::uci::GoCommandOptions& goOptions);
};

} // namespace ben_bot::search
