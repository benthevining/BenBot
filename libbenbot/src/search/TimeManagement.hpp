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

// times the search and also watches the "exit" flag
struct Interrupter final {
    Interrupter(
        const std::atomic_bool&           exitFlagToUse,
        const std::optional<Milliseconds> maxSearchTime)
        : exitFlag { exitFlagToUse }
        , searchTime { maxSearchTime }
    {
    }

    [[nodiscard]] Milliseconds get_search_duration() const
    {
        return std::chrono::duration_cast<Milliseconds>(Clock::now() - startTime);
    }

    [[nodiscard]] bool should_abort()
    {
        aborted = aborted || should_trigger_abort();

        return aborted;
    }

    [[nodiscard]] bool was_aborted() const noexcept { return aborted; }

private:
    [[nodiscard]] bool should_trigger_abort() const
    {
        if (exitFlag.load())
            return true;

        if (! searchTime.has_value())
            return false;

        return get_search_duration() >= *searchTime;
    }

    // On some systems, high_resolution_clock can be unsteady,
    // in which case it's better to fall back to steady_clock
    using Clock = std::conditional_t<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock>;

    const std::atomic_bool& exitFlag;

    std::chrono::time_point<Clock> startTime { Clock::now() };

    std::optional<Milliseconds> searchTime;

    bool aborted { false };
};

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
