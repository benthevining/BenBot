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
    This file defines the searcher thread class.
    @ingroup search
 */

#pragma once

#include <atomic>
#include <libbenbot/search/Search.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/util/Threading.hpp>
#include <thread>
#include <utility>

namespace chess::uci {
struct GoCommandOptions;
} // namespace chess::uci

namespace ben_bot::search {

/** A worker thread that can be used to execute a search.

    @ingroup search
 */
struct Thread final {
    /** Creates a searcher thread with a specified set of result callbacks.
        Note that the result callbacks will be invoked on the background thread.
     */
    explicit Thread(Callbacks&& callbacksToUse)
        : context { std::move(callbacksToUse) }
    {
    }

    ~Thread()
    {
        threadShouldExit.store(true);
        context.abort();
        searcherThread.join();
    }

    Thread(const Thread&)            = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&&)                 = delete;
    Thread& operator=(Thread&&)      = delete;

    /** The context used for this thread's searches.
        Note that the background thread will reference this object, so it should
        only be modified after calling ``context.wait()``.
     */
    Context context;

    /** Sets the position to be searched by the next search invocation.
        This method blocks waiting for any previously executing search to complete.
     */
    void set_position(const Position& pos)
    {
        context.wait();

        context.options.position = pos;

        // clear this so that all legal moves will be searched by default
        context.options.movesToSearch.clear();
    }

    /** Begins searching asynchronously.
        This method returns immediately, and the actual search will be performed by
        a background thread.
     */
    /// @{
    void start(chess::uci::GoCommandOptions&& options)
    {
        context.pondering.store(options.ponderMode);
      
        context.wait(); // shouldn't have been searching, but better safe than sorry

        context.options.update_from(std::move(options));

        startSearch.store(true);
    }

    void start()
    {
        context.wait(); // shouldn't have been searching, but better safe than sorry

        startSearch.store(true);
    }
    /// @}

private:
    void thread_func()
    {
        while (true) {
            // we want to use progressive backoff to wait on the startSearch flag,
            // but we also need to exit the PB loop if the threadShouldExit flag
            // gets set
            chess::util::progressive_backoff([this] {
                return threadShouldExit.load() or startSearch.exchange(false);
            });

            if (threadShouldExit.load()) {
                [[unlikely]];
                return;
            }

            context.search();
        }
    }

    std::thread searcherThread { [this] { thread_func(); } };

    std::atomic_bool threadShouldExit { false };
    std::atomic_bool startSearch { false };
};

} // namespace ben_bot::search
