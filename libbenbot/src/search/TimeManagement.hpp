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

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <optional>
#include <type_traits>

namespace ben_bot::search {

using std::chrono::milliseconds;
using std::size_t;

// simple RAII timer that measures the amount of time it's been alive
struct Timer final {
    [[nodiscard]] auto get_duration() const noexcept -> milliseconds
    {
        return std::chrono::duration_cast<milliseconds>(Clock::now() - startTime);
    }

private:
    // on some systems, high_resolution_clock can be unsteady,
    // in which case it's better to fall back to steady_clock
    using Clock = std::conditional_t<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock>;

    std::chrono::time_point<Clock> startTime { Clock::now() };
};

// this object is responsible for interrupting an ongoing search
// monitors the search's duration, and also watches the exit flag
struct Interrupter final {
    Interrupter(
        std::atomic_bool&                 exitFlagToUse,
        const std::atomic_bool&           ponderFlagToUse,
        const std::optional<milliseconds> maxSearchTime,
        const bool                        infinite)
        : exitFlag { exitFlagToUse }
        , ponderFlag { ponderFlagToUse }
        , searchTime { maxSearchTime }
        , infiniteMode { infinite }
    {
        // make sure exit flag is false when search starts
        exitFlagToUse.store(false);
    }

    [[nodiscard]] auto get_search_duration() const noexcept -> milliseconds { return timer.get_duration(); }

    // returns time remaining until abort time, or nullopt if there's no time bound
    [[nodiscard]] auto get_remaining_time() const -> std::optional<milliseconds>
    {
        return searchTime.transform([this](const milliseconds timeLimit) {
            return timeLimit - get_search_duration();
        });
    }

    // "active" check: queries clock time to check search duration, checks atomic stop flag
    // updates cached internal abort state
    [[nodiscard]] auto should_abort() noexcept -> bool
    {
        aborted = aborted or should_trigger_abort();

        return aborted;
    }

    // "passive" check: only returns cached internal abort state
    [[nodiscard]] auto was_aborted() const noexcept -> bool { return aborted; }

    void iteration_completed() noexcept { anyIterationCompleted = true; }

private:
    [[nodiscard]] auto should_trigger_abort() const noexcept -> bool
    {
        // we don't allow aborting until at least the depth 1 search has been completed
        if (not anyIterationCompleted)
            return false;

        if (exitFlag.load())
            return true;

        // don't exit the search when in ponder mode
        if (infiniteMode or ponderFlag.load())
            return false;

        return searchTime
            .transform([this](const milliseconds timeLimit) {
                return get_search_duration() >= timeLimit;
            })
            .value_or(false);
    }

    const std::atomic_bool& exitFlag;   // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    const std::atomic_bool& ponderFlag; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

    Timer timer;

    std::optional<milliseconds> searchTime;

    // because checking the clock's current time is probably a system call,
    // it's desirable to try and cache the aborted state to avoid recalculating
    // it when possible
    bool aborted { false };

    bool anyIterationCompleted { false };

    bool infiniteMode { false };
};

} // namespace ben_bot::search
