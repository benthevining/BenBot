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

#include <cassert>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/search/Options.hpp>
#include <libchess/uci/CommandParsing.hpp>
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

        assert(movesToGo > 0uz);

        return milliseconds {
            (static_cast<size_t>(timeRemaining.count()) / movesToGo)
            + (static_cast<size_t>(inc.count()) / (movesToGo / 10uz))
        };
    }
} // namespace

void Options::update_from(const chess::uci::GoCommandOptions& goOptions)
{
    movesToSearch = goOptions.moves;
    maxNodes      = goOptions.nodes;
    infinite      = goOptions.infinite;
    mateIn        = goOptions.mateIn;

    if (goOptions.depth.has_value())
        depth = *goOptions.depth;

    // search time
    if (goOptions.searchTime.has_value()) {
        searchTime = goOptions.searchTime;
    } else if (goOptions.infinite) {
        searchTime = std::nullopt;
    } else {
        const bool isWhite = position.is_white_to_move();

        const auto& timeLeft = isWhite ? goOptions.whiteTimeLeft : goOptions.blackTimeLeft;

        // need to know at least our time remaining in order to calculate search time limit
        if (timeLeft.has_value()) {
            searchTime = determine_search_time(
                *timeLeft,
                isWhite ? goOptions.whiteInc : goOptions.blackInc,
                goOptions.movesToGo);
        }
    }
}

} // namespace ben_bot::search
