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
#include <cmath>
#include <format>
#include <iostream>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Chrono.hpp>
#include <libutil/Variant.hpp>
#include <optional>
#include <print>
#include <span>
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

    return std::monostate { };
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
            .value_or(string { });
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

auto SearchInfo::Score::MateIn::moves() const noexcept -> int
{
    return [ply = plies]() mutable {
        if (std::cmp_greater(ply, 0))
            ++ply;

        return ply / 2;
    }();
}

auto SearchInfo::get_nps() const noexcept -> size_t
{
    using FractionalSeconds = util::FractionalDuration<std::chrono::seconds, double>;

    const auto seconds = duration_cast<FractionalSeconds>(time).count();

    if (seconds <= 0.)
        return 0uz;

    const auto nps = static_cast<double>(nodes) / seconds;

    return static_cast<size_t>(std::round(nps));
}

namespace {
    using Score = SearchInfo::Score;

    [[nodiscard]] auto base_score_string(const Score& score) -> string
    {
        return std::visit(
            util::Visitor {
                [](const Score::Centipawns& centipawns) { return std::format("cp {}", centipawns.value); },
                [](const Score::MateIn& mate) { return std::format("mate {}", mate.moves()); } },
            score.value);
    }

    [[nodiscard]] auto score_string(const Score& score) -> string
    {
        auto string = base_score_string(score);

        assert(not(score.lowerBound and score.upperBound));

        if (score.lowerBound)
            string.append(" lowerbound");
        else if (score.upperBound)
            string.append(" upperbound");

        return string;
    }

    [[nodiscard]] auto pv_string(const std::span<const Move> pv) -> string
    {
        if (pv.empty()) {
            // this is possible if we're checkmated
            return { };
        }

        string result { " pv" };

        for (const auto move : pv)
            result.append(std::format(" {}", to_uci(move)));

        return result;
    }

    [[nodiscard]] auto get_extra_info_string(const string_view info) -> string
    {
        if (info.empty())
            return { };

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
        info.get_nps(),
        info.selDepth, info.tbHits,
        pv_string(info.pv),
        get_extra_info_string(info.extraInformation));

    cout.flush();
}

} // namespace chess::uci::printing
