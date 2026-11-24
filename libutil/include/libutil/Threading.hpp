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
    This file provides some threading and synchronization utilities.
    @ingroup util
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <functional>
#include <memory>

namespace util {

/** A thread with a stack of a chosen size.

    ``std::thread`` always uses the platform's default stack size for new threads,
    which is only 512 KiB on macOS (and 1 MiB on Windows), so code that recurses
    deeply, like a chess search, should use this class instead.

    The thread starts running the function as soon as it's constructed. It must be
    joined before it's destroyed.

    @ingroup util
 */
class LargeStackThread final {
public:
    /** Starts a new thread with a stack of at least ``stackSize`` bytes,
        which runs ``func``. ``stackSize`` must be a multiple of 64 KiB.

        @throws std::system_error if the thread can't be created
     */
    LargeStackThread(std::size_t stackSize, std::function<void()> func);

    ~LargeStackThread();

    LargeStackThread(const LargeStackThread&)            = delete;
    LargeStackThread& operator=(const LargeStackThread&) = delete;
    LargeStackThread(LargeStackThread&&)                 = delete;
    LargeStackThread& operator=(LargeStackThread&&)      = delete;

    /** Blocks until the thread's function returns. This must be called exactly once. */
    void join();

private:
    struct Impl;

    std::unique_ptr<Impl> impl;
};

/** Blocks the calling thread until ``pred`` returns true.
    Blocking is implemented by spinning on the predicate and
    using a progressive backoff strategy.

    This progressive backoff strategy avoids wasting energy,
    and allows other threads to make progress by yielding the
    waiting thread after a certain amount of time. This time
    is chosen to be about 1 millisecond.

    On platforms other than x86, x86_64, or arm64, a simple
    implementation will be used.

    @ingroup util
 */
void progressive_backoff(std::function<bool()> pred);

} // namespace util
