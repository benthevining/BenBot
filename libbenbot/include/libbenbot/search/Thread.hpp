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
#include <chrono>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Context.hpp>
#include <libchess/game/Position.hpp>
#include <thread>

namespace chess::uci {
struct GoCommandOptions;
} // namespace chess::uci

namespace ben_bot::search {

using std::chrono::milliseconds;

/** A worker thread that can be used to execute a search.

    @ingroup search
 */
struct Thread final {
    /** Creates a searcher thread with an empty set of result callbacks. */
    Thread() = default;

    /** Creates a searcher thread with a specified set of result callbacks.
        Note that the result callbacks will be invoked on the background thread.
     */
    explicit Thread(Callbacks&& callbacksToUse);

    ~Thread();

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
    void set_position(const Position& pos);

    /** Begins searching asynchronously.
        This method returns immediately, and the actual search will be performed by
        a background thread.
     */
    /// @{
    void start(
        const chess::uci::GoCommandOptions& options,
        milliseconds                        moveOverheadTime = milliseconds { 0uz });

    void start();
    /// @}

private:
    void thread_func();

    std::thread searcherThread { [this] { thread_func(); } };

    std::atomic_bool threadShouldExit { false };
    std::atomic_bool startSearch { false };
};

} // namespace ben_bot::search
