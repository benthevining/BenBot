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
    This file provides a small parallel-for helper for the tuner.
    @ingroup tuner
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <exception>
#include <thread>
#include <vector>

namespace ben_bot::tuner {

using std::size_t;

/** Returns the number of threads to use by default.
    @ingroup tuner
 */
[[nodiscard]] inline auto default_num_threads() noexcept -> size_t
{
    return std::max(1uz, static_cast<size_t>(std::thread::hardware_concurrency()));
}

/** Splits ``[0, count)`` into ``numThreads`` contiguous chunks and calls
    ``func(threadIndex, begin, end)`` for each chunk, on its own thread.

    The chunks are fixed by ``count`` & ``numThreads``, so a caller that reduces
    per-thread results in thread order gets the same answer on every run.
    If any call throws, the first exception (in thread order) is rethrown once all
    threads have finished.

    @ingroup tuner
 */
template <typename Func>
void parallel_for(size_t numThreads, const size_t count, Func&& func)
{
    numThreads = std::clamp(numThreads, 1uz, std::max(1uz, count));

    const auto chunk_begin = [count, numThreads](const size_t index) {
        return (count * index) / numThreads;
    };

    if (numThreads == 1uz) {
        func(0uz, 0uz, count);
        return;
    }

    std::vector<std::exception_ptr> errors(numThreads);

    {
        std::vector<std::thread> threads;

        // std::jthread isn't in every standard library that CI builds with yet (Apple's libc++ on
        // macos-15 doesn't have it), so this joins the threads itself, even if starting one throws
        struct Joiner final {
            std::vector<std::thread>& toJoin;

            explicit Joiner(std::vector<std::thread>& threadsToJoin) noexcept
                : toJoin { threadsToJoin }
            {
            }

            Joiner(const Joiner&)            = delete;
            Joiner& operator=(const Joiner&) = delete;

            ~Joiner()
            {
                for (auto& thread : toJoin)
                    if (thread.joinable())
                        thread.join();
            }
        } joiner { threads };

        threads.reserve(numThreads);

        for (auto i = 0uz; i < numThreads; ++i) {
            threads.emplace_back([&func, &errors, &chunk_begin, i] {
                try {
                    func(i, chunk_begin(i), chunk_begin(i + 1uz));
                } catch (...) {
                    errors.at(i) = std::current_exception();
                }
            });
        }
    } // the threads join here

    for (const auto& error : errors)
        if (error)
            std::rethrow_exception(error);
}

} // namespace ben_bot::tuner
