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
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Options.hpp>
#include <utility>

namespace ben_bot::search {

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

    /** The transposition table used for this search.
        This object's methods can only be safely called when no search is executing.
     */
    TranspositionTable transTable;

    /** Performs a search.
        Results will be propagated via the ``callbacks`` that have been
        assigned.

        The search may execute for a potentially unbounded amount of time.
        The search can be interrupted by calling the ``abort()`` method while
        ``search()`` is executing.

        This function accesses ``options`` and ``transTable``; these objects
        must not be mutated while ``search()`` is executing. ``abort()``,
        ``wait()``, ``in_progress()``, and ``reset()`` may be called while
        ``search()`` is executing without introducing data races.
     */
    void search();

    /** This function may be called while ``search()`` is executing to interrupt
        the search. If a search is in progress, calling this method will cause the
        search routine to return at the next available point. This method returns
        immediately; to wait for the search to complete, call ``wait()`` after
        calling this method.
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

    /** Sets whether the next search will be in ponder mode.
        When in ponder mode, the search will not exit until ``abort()`` or
        ``ponder_hit()`` are called.
     */
    void set_pondering(bool isPonderMode) noexcept { pondering.store(isPonderMode); }

    /** When in a ponder mode search, this exits the search. A ponder mode search
        will not exit until this method or ``abort()`` are called.
     */
    void ponder_hit() noexcept { pondering.store(false); }

private:
    std::atomic_bool exitFlag { false };

    std::atomic_bool activeFlag { false };

    std::atomic_bool pondering { false };

    Callbacks callbacks;
};

} // namespace ben_bot::search
