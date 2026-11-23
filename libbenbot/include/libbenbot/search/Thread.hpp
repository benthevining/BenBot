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
#include <libutil/Memory.hpp>
#include <libutil/Threading.hpp>

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

    /** Begins searching asynchronously.
        This method returns immediately, and the actual search will be performed by
        a background thread.
     */
    void start()
    {
        // shouldn't have been searching, but better safe than sorry
        context.abort();
        context.wait();

        // reset the abort flag here, rather than on the background thread, so that
        // a stop command received before the background thread begins searching isn't lost
        context.reset_abort();

        startSearch.store(true, std::memory_order::release);
    }

private:
    void thread_func();

    // align to cache lines to prevent false sharing of these flags
    alignas(util::memory::CacheLineSize) std::atomic_bool threadShouldExit { false };
    alignas(util::memory::CacheLineSize) std::atomic_bool startSearch { false };

    // The search recurses once per ply, up to MAX_PLY plies deep, & each ply uses about 3 KiB
    // of stack in a Release build (more in Debug & sanitizer builds), so it can need more than
    // the default stack size for a new thread, which is only 512 KiB on macOS. The memory is
    // only reserved, so a generous size costs nothing until the search actually goes that deep.
    static constexpr auto STACK_SIZE { 64uz * 1024uz * 1024uz };

    // this must be declared after everything that thread_func() uses, since members are
    // initialized in declaration order and the thread starts running as soon as it's created
    util::LargeStackThread searcherThread { STACK_SIZE, [this] { thread_func(); } };
};

} // namespace ben_bot::search
