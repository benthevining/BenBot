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
    This file defines the search callbacks struct.
    @ingroup search
 */

#pragma once

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <functional>
#include <libbenbot/eval/Score.hpp>
#include <libchess/moves/Move.hpp>

namespace ben_bot::search {

using chess::moves::Move;
using std::chrono::milliseconds;
using std::size_t;

/** This struct encapsulates a set of functions that will be called to
    process search progress and results. Search results are always
    retrieved through these callbacks.

    @ingroup search
    @see Context
 */
struct Callbacks final {
    /** The results from a completed search. */
    struct Result final {
        /** The total amount of time spent searching to produce this result.
            For depths greater than 1, this value is the duration of the entire
            search, including lower depths of the iterative deepening loop.
         */
        milliseconds duration { 0uz };

        /** The total depth that was searched. */
        size_t depth { 0uz };

        /** The evaluation of the position resulting from playing the best move. */
        eval::Score score;

        /** The best move found in the position. */
        Move bestMove;

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

        /** Number of cutoffs due to mate distance pruning. */
        size_t mdpCutoffs { 0uz };
    };

    /** Function type that accepts a single Result argument. */
    using Callback = std::function<void(const Result&)>;

    /** Function object that will be invoked with results from a completed search. */
    Callback onSearchComplete;

    /** Function object that will be invoked with results from each iteration of
        the iterative deepening loop.
     */
    Callback onIteration;

    /** Can be safely called without checking if ``onSearchComplete`` is null. */
    void search_complete(const Result& result) const
    {
        if (onSearchComplete != nullptr)
            onSearchComplete(result);
    }

    /** Can be safely called without checking if ``onIteration`` is null. */
    void iteration_complete(const Result& result) const
    {
        if (onIteration != nullptr)
            onIteration(result);
    }
};

} // namespace ben_bot::search
