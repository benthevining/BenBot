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
    This file defines the search result type.
    @ingroup search
 */

#pragma once

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t;
#include <libbenbot/eval/Score.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <optional>

namespace chess::uci::printing {
struct SearchInfo;
} // namespace chess::uci::printing

namespace ben_bot::search {

using chess::moves::Move;
using chess::moves::MoveList;
using std::chrono::milliseconds;
using std::size_t;

/** The results from a completed search.
    This struct can also represent a still-in-progress search (ie, an update
    on each iterative deepening iteration).
 */
struct [[nodiscard]] Result final {
    /** The total amount of time spent searching to produce this result.
        For depths greater than 1, this value is the duration of the entire
        search, including lower depths of the iterative deepening loop.
     */
    milliseconds duration { 0uz };

    /** The total depth that was searched.
        This value is the highest depth that was completed.
     */
    size_t depth { 0uz };

    /** The deepest depth reached by the quiescence search. */
    size_t qDepth { 0uz };

    /** The evaluation of the position resulting from playing the best move. */
    eval::Score score;

    /** The principal variation found during the search. */
    MoveList pv;

    /** The total number of nodes visited during this search. For depths greater
        than 1, this value includes nodes visited in shallower depths of the
        iterative deepening loop.
     */
    size_t nodesSearched { 0uz };

    /** The number of nodes for which a saved score from the transposition table
        could be used.
     */
    size_t transpositionTableHits { 0uz };

    /** Number of cutoffs due to alpha-beta. */
    size_t betaCutoffs { 0uz };

    /** The total number of static evaluations performed during the search. */
    size_t staticEvals { 0uz };

    /** Number of cutoffs due to mate distance pruning. */
    size_t mdpCutoffs { 0uz };

    /** The approximate fraction of transposition table entries (permille) that
        were written to during this root search.
     */
    size_t hashfull { 0uz };

    /** Returns the best move found by this search. */
    [[nodiscard]] Move best_move() const { return pv.front(); }

    /** Returns the ponder move found by this search, if one exists. */
    [[nodiscard]] std::optional<Move> ponder_move() const
    {
        if (pv.size() < 2uz)
            return std::nullopt;

        return pv[1uz];
    }

    /** Typedef for the libchess type used for printing UCI-formatted output. */
    using LibchessResult = chess::uci::printing::SearchInfo;

    /** Converts this object to the libchess type used for printing UCI-formatted output. */
    [[nodiscard]] auto to_libchess(bool includeDebugInfo) const -> LibchessResult;
};

} // namespace ben_bot::search
