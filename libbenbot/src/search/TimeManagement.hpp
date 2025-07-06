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
#include <cassert>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <optional>
#include <type_traits>

namespace chess::search {

using Milliseconds = std::chrono::milliseconds;

using std::size_t;

// simple RAII timer that measures the amount of time it's been alive
struct Timer final {
    [[nodiscard]] Milliseconds get_duration() const
    {
        return std::chrono::duration_cast<Milliseconds>(Clock::now() - startTime);
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
        const std::optional<Milliseconds> maxSearchTime)
        : exitFlag { exitFlagToUse }
        , searchTime { maxSearchTime }
    {
        // make sure exit flag is false when search starts
        exitFlagToUse.store(false);
    }

    [[nodiscard]] Milliseconds get_search_duration() const { return timer.get_duration(); }

    // returns time remaining until abort time, or nullopt if there's no time bound
    [[nodiscard]] std::optional<Milliseconds> get_remaining_time() const
    {
        return searchTime.and_then([this](const Milliseconds timeLimit) {
            return std::optional { timeLimit - get_search_duration() };
        });
    }

    // "active" check: queries clock time to check search duration, checks atomic stop flag
    // updates cached internal abort state
    [[nodiscard]] bool should_abort()
    {
        aborted = aborted || should_trigger_abort();

        return aborted;
    }

    // "passive" check: only returns cached internal abort state
    [[nodiscard]] bool was_aborted() const noexcept { return aborted; }

    void iteration_completed() noexcept { anyIterationCompleted = true; }

private:
    [[nodiscard]] bool should_trigger_abort() const
    {
        // we don't allow aborting until at least the depth 1 search has been completed
        if (! anyIterationCompleted)
            return false;

        if (exitFlag.load())
            return true;

        if (! searchTime.has_value())
            return false;

        return get_search_duration() >= *searchTime;
    }

    const std::atomic_bool& exitFlag; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

    Timer timer;

    std::optional<Milliseconds> searchTime;

    // because checking the clock's current time is probably a system call,
    // it's desirable to try and cache the aborted state to avoid recalculating
    // it when possible
    bool aborted { false };

    bool anyIterationCompleted { false };
};

// decides the amount of time to limit the search to, based on the parameters
[[nodiscard, gnu::const]] inline Milliseconds determine_search_time(
    const Milliseconds                timeRemaining,
    const std::optional<Milliseconds> increment,
    const std::optional<size_t>       movesToNextTimeControl)
{
    const auto inc = increment.value_or(Milliseconds { 0 });

    const auto movesToGo = movesToNextTimeControl.value_or(40uz);

    assert(movesToGo > 0uz);

    return Milliseconds {
        (timeRemaining.count() / movesToGo)
        + (inc.count() / (movesToGo / 10uz))
    };
}

} // namespace chess::search
