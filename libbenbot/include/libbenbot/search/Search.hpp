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
#include <libbenbot/search/TranspositionTable.hpp>
#include <libchess/moves/Move.hpp>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::uci {
struct GoCommandOptions;
} // namespace chess::uci

/** This namespace contains functions related to searching the
    move tree for the principal variation.

    @ingroup search
 */
namespace chess::search {

using game::Position;
using moves::Move;
using std::size_t;

using Milliseconds = std::chrono::milliseconds;

#ifndef DOXYGEN
namespace detail {
    [[nodiscard, gnu::const]] bool is_mate_score(int score) noexcept;

    [[nodiscard, gnu::const]] size_t ply_to_mate_from_score(int score) noexcept;
} // namespace detail
#endif

/** This struct encapsulates the parameters to the search algorithm.

    @ingroup search
    @see Context
 */
struct Options final {
    /** The root position to be searched. */
    Position position;

    /** The maximum search depth (in plies). */
    size_t depth { std::numeric_limits<size_t>::max() };

    /** The maximum search time. */
    std::optional<Milliseconds> searchTime;

    /** Search only this many nodes. Setting this value too low can
        really emphasize the effects of our move ordering algorithm.
     */
    std::optional<size_t> maxNodes;

    /** Restrict the search to only these moves.
        If this is empty, all legal moves in the position will be searched.
     */
    std::vector<Move> movesToSearch;

    /** Updates the values in this options struct with the UCI "go" command options. */
    void update_from(uci::GoCommandOptions&& goOptions);

    /** Returns true if this search has any bounds other than depth. */
    [[nodiscard]] bool is_bounded() const noexcept
    {
        return searchTime.has_value() || maxNodes.has_value();
    }
};

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
        Milliseconds duration { 0uz };

        /** The total depth that was searched. */
        size_t depth { 0uz };

        /** The evaluation of the position resulting from playing the best move. */
        int score { 0 };

        /** The best move found in the position. */
        Move bestMove;
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

    /** Creates a set of callbacks that print UCI-compatible output. */
    [[nodiscard]] static Callbacks make_uci_handler();
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
        If a search is in progress, this method cancels it and blocks until it returns.
        Invoking this method is thread-safe, even if a search was in progress.
     */
    void reset()
    {
        abort();
        wait();
        transTable.clear();
    }

    /** Returns true if a search is currently in progress. */
    [[nodiscard]] bool in_progress() const noexcept { return activeFlag.load(); }

    /** Blocks the calling thread until the search in progress is complete.
        Returns immediately if no search was in progress when this function was called.
     */
    void wait() const;

private:
    std::atomic_bool exitFlag { false };

    std::atomic_bool activeFlag { false };

    TranspositionTable transTable;

    Callbacks callbacks;
};

} // namespace chess::search
