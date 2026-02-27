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

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/search/Options.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <limits>
#include <optional>

namespace ben_bot::search {

namespace {
    using std::chrono::milliseconds;
    using std::size_t;

    [[nodiscard, gnu::const]] constexpr auto determine_search_time(
        const milliseconds                timeRemaining,
        const std::optional<milliseconds> increment,
        const std::optional<size_t>       movesToNextTimeControl)
        -> milliseconds
    {
        const auto inc = increment.value_or(milliseconds { 0 });

        const auto movesToGo = movesToNextTimeControl.value_or(40uz);

        if (movesToGo <= 1uz)
            return timeRemaining + inc;

        return milliseconds {
            (static_cast<size_t>(timeRemaining.count()) / movesToGo)
            + (static_cast<size_t>(inc.count()) / (movesToGo / 10uz))
        };
    }
} // namespace

void Options::update_from(const chess::uci::GoCommandOptions& goOptions)
{
    movesToSearch = goOptions.moves;
    infinite      = goOptions.infinite;
    mateIn        = goOptions.mateIn;

    static constexpr auto MAX = std::numeric_limits<size_t>::max();

    depth    = goOptions.depth.value_or(MAX);
    maxNodes = goOptions.nodes.value_or(MAX);

    searchTime = goOptions.searchTime
                     .or_else([&goOptions, isWhite = position.is_white_to_move()]() -> std::optional<milliseconds> {
                         if (goOptions.infinite)
                             return std::nullopt;

                         // need to know at least our time remaining in order to calculate search time limit
                         const auto& timeLeft = isWhite ? goOptions.whiteTimeLeft : goOptions.blackTimeLeft;

                         return timeLeft
                             .transform([&goOptions, isWhite](const milliseconds msLeft) {
                                 return determine_search_time(
                                     msLeft,
                                     isWhite ? goOptions.whiteInc : goOptions.blackInc,
                                     goOptions.movesToGo);
                             });
                     })
                     .transform([overhead = moveOverhead](const milliseconds time) {
                         return time - overhead;
                     });
}

} // namespace ben_bot::search
