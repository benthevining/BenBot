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

auto Options::from_libchess(
    const chess::uci::GoCommandOptions& goOptions,
    const bool                          isWhiteToMove) -> Options
{
    Options opts { };

    opts.movesToSearch = goOptions.moves;
    opts.infinite      = goOptions.infinite;
    opts.mateIn        = goOptions.mateIn;

    static constexpr auto MAX = std::numeric_limits<size_t>::max();

    opts.depth    = goOptions.depth.value_or(MAX);
    opts.maxNodes = goOptions.nodes.value_or(MAX);

    opts.searchTime = goOptions.searchTime
                          .or_else([isWhiteToMove, &goOptions] -> std::optional<milliseconds> {
                              if (goOptions.infinite)
                                  return std::nullopt;

                              // need to know at least our time remaining in order to calculate search time limit
                              const auto& timeLeft = isWhiteToMove ? goOptions.whiteTimeLeft : goOptions.blackTimeLeft;

                              return timeLeft
                                  .transform([isWhiteToMove, &goOptions](const milliseconds msLeft) {
                                      return determine_search_time(
                                          msLeft,
                                          isWhiteToMove ? goOptions.whiteInc : goOptions.blackInc,
                                          goOptions.movesToGo);
                                  });
                          })
                          .transform([overhead = opts.moveOverhead](const milliseconds time) {
                              return time - overhead;
                          });

    return opts;
}

} // namespace ben_bot::search
