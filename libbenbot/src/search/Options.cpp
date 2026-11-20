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

#include <algorithm>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <iterator>
#include <libbenbot/search/Options.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <limits>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace ben_bot::search {

namespace {
    using std::chrono::milliseconds;
    using std::size_t;

    struct TimeLimits final {
        milliseconds soft;
        milliseconds hard;
    };

    // number of moves we assume are left in the game if we aren't told
    inline constexpr auto DEFAULT_MOVES_TO_GO { 30uz };

    [[nodiscard, gnu::const]] constexpr auto determine_search_time(
        const milliseconds                timeRemaining,
        const std::optional<milliseconds> increment,
        const std::optional<size_t>       movesToNextTimeControl,
        const milliseconds                overhead)
        -> TimeLimits
    {
        // never plan to use more time than we have
        const auto available = std::max(timeRemaining - overhead, milliseconds { 1 });

        const auto inc = increment.value_or(milliseconds { 0 });

        const auto movesToGo = static_cast<milliseconds::rep>(
            std::max(movesToNextTimeControl.value_or(DEFAULT_MOVES_TO_GO), 1uz));

        // the soft limit is our share of the remaining time, plus most of the increment,
        // but never more than half the clock (even if this is the last move before the time control)
        const auto soft = std::min(
            (available / movesToGo) + (inc * 3 / 4),
            available / 2);

        // the hard limit lets an iteration that's already started run on for a while, but
        // never past twice our share of the clock, or 3/4 of the clock (whichever is less)
        const auto hard = std::max(
            soft,
            std::min({ soft * 3, available * 2 / movesToGo, available * 3 / 4 }));

        return { .soft = std::max(soft, milliseconds { 1 }), .hard = std::max(hard, milliseconds { 1 }) };
    }
} // namespace

auto Options::from_libchess(
    const chess::uci::GoCommandOptions& goOptions,
    const bool                          isWhiteToMove,
    const milliseconds                  overhead) -> Options
{
    Options opts { };

    opts.movesToSearch = goOptions.moves;
    opts.infinite      = goOptions.infinite;
    opts.mateIn        = goOptions.mateIn;

    static constexpr auto MAX = std::numeric_limits<size_t>::max();

    opts.depth    = goOptions.depth.value_or(MAX);
    opts.maxNodes = goOptions.nodes.value_or(MAX);

    opts.moveOverhead = overhead;

    // "go movetime": search for exactly this long
    if (goOptions.searchTime.has_value()) {
        opts.searchTime = std::max(*goOptions.searchTime - overhead, milliseconds { 1 });
        return opts;
    }

    if (goOptions.infinite)
        return opts;

    // need to know at least our time remaining in order to calculate search time limits
    const auto& timeLeft = isWhiteToMove ? goOptions.whiteTimeLeft : goOptions.blackTimeLeft;

    if (timeLeft.has_value()) {
        const auto limits = determine_search_time(
            *timeLeft,
            isWhiteToMove ? goOptions.whiteInc : goOptions.blackInc,
            goOptions.movesToGo,
            overhead);

        opts.searchTime     = limits.hard;
        opts.softSearchTime = limits.soft;
    }

    return opts;
}

using chess::game::Position;
using chess::moves::Move;
using chess::moves::MoveList;
using nlohmann::json;
using std::string_view;

namespace {
    inline constexpr string_view TAG_OPT_HAS_VALUE { "has_value" };
    inline constexpr string_view TAG_OPT_VALUE { "value" };

    template <typename T>
    [[nodiscard]] auto opt_to_json(const std::optional<T>& opt) -> json
    {
        json data;

        data[TAG_OPT_HAS_VALUE] = opt.has_value();

        if (opt.has_value())
            data[TAG_OPT_VALUE] = opt.value();

        return data;
    }

    template <typename T>
    [[nodiscard]] auto opt_from_json(const json& data) -> std::optional<T>
    {
        if (data.contains(TAG_OPT_HAS_VALUE))
            if (not data.at(TAG_OPT_HAS_VALUE).get<bool>())
                return std::nullopt;

        if (not data.contains(TAG_OPT_VALUE))
            return std::nullopt;

        return data.at(TAG_OPT_VALUE).get<T>();
    }

    [[nodiscard]] auto moves_to_json(const MoveList& moves) -> json
    {
        auto data = json::array();

        std::ranges::transform(
            moves,
            std::back_inserter(data),
            chess::notation::to_uci);

        return data;
    }

    [[nodiscard]] auto moves_from_json(
        const json& data, const Position& position) -> MoveList
    {
        MoveList moves;

        for (const auto& value : data) {
            [[maybe_unused]] const auto result
                = chess::notation::from_uci(
                    position,
                    value.get<string_view>())
                      .transform([&moves](const Move move) {
                          moves.push_back(move);
                          return std::monostate { };
                      });
        }

        return moves;
    }
} // namespace

inline constexpr string_view TAG_DEPTH { "depth" };
inline constexpr string_view TAG_SEARCH_TIME { "search_time" };
inline constexpr string_view TAG_SOFT_SEARCH_TIME { "soft_search_time" };
inline constexpr string_view TAG_MOVE_OVERHEAD { "move_overhead" };
inline constexpr string_view TAG_MAX_NODES { "max_nodes" };
inline constexpr string_view TAG_MOVES_TO_SEARCH { "moves_to_search" };
inline constexpr string_view TAG_INFINITE { "infinite" };
inline constexpr string_view TAG_MATE_IN { "mate_in" };
inline constexpr string_view TAG_MULTI_PV { "multi_pv" };

auto Options::to_string() const -> std::string
{
    json data;

    data[TAG_DEPTH]            = depth;
    data[TAG_SEARCH_TIME]      = opt_to_json(searchTime.transform(&milliseconds::count));
    data[TAG_SOFT_SEARCH_TIME] = opt_to_json(softSearchTime.transform(&milliseconds::count));
    data[TAG_MOVE_OVERHEAD]    = moveOverhead.count();
    data[TAG_MAX_NODES]        = maxNodes;
    data[TAG_MOVES_TO_SEARCH]  = moves_to_json(movesToSearch);
    data[TAG_INFINITE]         = infinite;
    data[TAG_MATE_IN]          = opt_to_json(mateIn);
    data[TAG_MULTI_PV]         = multiPV;

    return data.dump();
}

auto Options::from_string(
    const string_view text,
    const Position&   currentPosition) -> Options
{
    const auto parsed = json::parse(text);

    return {
        .depth      = parsed.at(TAG_DEPTH).get<size_t>(),
        .searchTime = opt_from_json<size_t>(parsed.at(TAG_SEARCH_TIME))
            .transform([](const size_t count) { return milliseconds { count }; }),
        // older saved states may not contain this field
        .softSearchTime = parsed.contains(TAG_SOFT_SEARCH_TIME)
                            ? opt_from_json<size_t>(parsed.at(TAG_SOFT_SEARCH_TIME))
                                  .transform([](const size_t count) { return milliseconds { count }; })
                            : std::nullopt,
        .moveOverhead  = milliseconds { parsed.at(TAG_MOVE_OVERHEAD).get<size_t>() },
        .maxNodes      = parsed.at(TAG_MAX_NODES).get<size_t>(),
        .movesToSearch = moves_from_json(
            parsed.at(TAG_MOVES_TO_SEARCH),
            currentPosition),
        .infinite = parsed.at(TAG_INFINITE).get<bool>(),
        .mateIn   = opt_from_json<size_t>(parsed.at(TAG_MATE_IN)),
        // older saved states may not contain this field
        .multiPV = parsed.value(TAG_MULTI_PV, 1uz)
    };
}

} // namespace ben_bot::search
