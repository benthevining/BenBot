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

#include <cmath>
#include <format>
#include <iostream>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Variant.hpp>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace chess::uci::printing {

// NB. With our println calls, we explicitly provide std::cout as the first parameter, because
// when the first parameter is omitted, the default overload writes to the FILE* stdout, but we
// want to use C++ iostreams for all our I/O so that our logging facility works correctly

using notation::to_uci;
using std::cout;
using std::println;
using std::string;
using std::string_view;

using MaybeMove = std::optional<Move>;

std::monostate info_string(const string_view info)
{
    println(cout, "info string {}", info);

    return std::monostate {};
}

namespace {
    [[nodiscard]] auto ponder_move_string(
        const MaybeMove ponderMove)
        -> string
    {
        return ponderMove
            .transform([](const Move move) {
                return std::format(" ponder {}", to_uci(move));
            })
            .value_or(string {});
    }
} // namespace

void best_move(
    const Move bestMove, const MaybeMove ponderMove)
{
    println(cout,
        "bestmove {}{}",
        to_uci(bestMove), ponder_move_string(ponderMove));

    cout.flush();
}

namespace {
    [[nodiscard, gnu::const]] auto plies_to_moves(int plies) noexcept -> int
    {
        if (std::cmp_greater(plies, 0))
            ++plies;

        return plies / 2;
    }

    using Score = SearchInfo::Score;

    [[nodiscard]] auto base_score_string(const Score& score) -> string
    {
        return std::visit(
            util::Visitor {
                [](const Score::Centipawns& centipawns) { return std::format("cp {}", centipawns.value); },
                [](const Score::MateIn& mate) { return std::format("mate {}", plies_to_moves(mate.plies)); } },
            score.value);
    }

    [[nodiscard]] auto score_string(const Score& score) -> string
    {
        auto string = base_score_string(score);

        if (score.lowerBound)
            string.append(" lowerbound");
        else if (score.upperBound)
            string.append(" upperbound");

        return string;
    }

    [[nodiscard]] auto pv_string(const moves::MoveList& pv) -> string
    {
        if (pv.empty()) {
            // this is possible if we're checkmated
            return {};
        }

        string result { " pv " };

        for (const auto move : pv) {
            result.append(to_uci(move));
            result.append(1uz, ' ');
        }

        result.pop_back(); // trim last space

        return result;
    }

    [[nodiscard, gnu::const]] auto get_nodes_per_second(const SearchInfo& info) -> size_t
    {
        const auto seconds = static_cast<double>(info.time.count()) * 0.001;

        if (seconds <= 0.)
            return 0uz;

        const auto nps = static_cast<double>(info.nodes) / seconds;

        return static_cast<size_t>(std::round(nps));
    }

    [[nodiscard]] auto get_extra_info_string(const string_view info) -> string
    {
        if (info.empty())
            return {};

        return std::format(" string {}", info);
    }
} // namespace

void search_info(const SearchInfo& info)
{
    println(cout,
        "info depth {} score {} time {} hashfull {} nodes {} nps {} seldepth {} tbhits {}{}{}",
        info.depth,
        score_string(info.score),
        info.time.count(), info.hashfull, info.nodes,
        get_nodes_per_second(info),
        info.selDepth, info.tbHits,
        pv_string(info.pv),
        get_extra_info_string(info.extraInformation));

    cout.flush();
}

} // namespace chess::uci::printing
