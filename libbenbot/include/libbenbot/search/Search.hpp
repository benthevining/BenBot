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
    This file defines the search context class.
    @ingroup search
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <functional>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/moves/Move.hpp>
#include <utility>

namespace ben_bot::search {

using chess::moves::Move;
using std::chrono::milliseconds;
using std::size_t;

/** This struct encapsulates a set of functions that will be called to
    process search progress and results.

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

    /** When a move from the opening book is played, this function object is called. */
    std::function<void(const Move&)> onOpeningBookHit;

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

    /** Can be safely called without checking if ``onOpeningBookHit`` is null. */
    void opening_book_hit(const Move& move) const
    {
        if (onOpeningBookHit != nullptr)
            onOpeningBookHit(move);
    }
};

/** This struct encapsulates everything needed to perform a search.
    You can keep one of these alive between searches by simply updating
    the options and then calling ``search()`` again.

    @ingroup search
 */
struct Context final {
    /** Creates a search context with a specified set of result callbacks. */
    explicit Context(Callbacks&& callbacksToUse)
        : callbacks { std::move(callbacksToUse) }
    {
    }

    /** The options to use for the search.
        This object can only be safely mutated when no search is executing.
     */
    Options options;

    /** The transposition table used for this search. */
    TranspositionTable transTable;

    /** Performs a search.
        Results will be propagated via the ``callbacks`` that have been
        assigned.

        The search may execute for a potentially unbounded amount of time.
        The search can be interrupted by calling the ``abort()`` method while
        ``search()`` is executing.

        This function accesses ``options`` and ``callbacks``; these objects
        must not be mutated while ``search()`` is executing. ``abort()``,
        ``wait()``, ``in_progress()``, and ``reset()`` may be called while
        ``search()`` is executing without introducing data races.
     */
    void search();

    /** This function may be called while ``search()`` is executing to interrupt
        the search. If a search is in progress, calling this method will cause the
        search routine to return at the next available point.
     */
    void abort() noexcept { exitFlag.store(true); }

    /** Clears the transposition table.
        If a search is in progress, this method blocks until it returns.
        Invoking this method is thread-safe, even if a search was in progress.
     */
    void clear_transposition_table()
    {
        wait();
        transTable.clear();
    }

    /** Returns true if a search is currently in progress. */
    [[nodiscard]] bool in_progress() const noexcept { return activeFlag.load(); }

    /** Blocks the calling thread until the search in progress is complete.
        Returns immediately if no search was in progress when this function was called.
     */
    void wait() const;

    /** This flag should be set to true during a ponder-mode search, and false otherwise. */
    std::atomic_bool pondering { false };

private:
    std::atomic_bool exitFlag { false };

    std::atomic_bool activeFlag { false };

    Callbacks callbacks;
};

} // namespace ben_bot::search
