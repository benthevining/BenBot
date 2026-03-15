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
using MoveSpan  = std::span<const Move>;

auto info_string(const string_view info) -> std::monostate
{
    println(cout, "info string {}", info);

    cout.flush();

    return std::monostate { };
}

void best_move(
    const Move bestMove, const MaybeMove ponderMove)
{
    const auto ponderMoveString = ponderMove
                                      .transform([](const Move move) {
                                          return std::format(" ponder {}", to_uci(move));
                                      })
                                      .value_or(string { });

    println(cout,
        "bestmove {}{}",
        to_uci(bestMove), ponderMoveString);

    cout.flush();
}

void currmove_info(
    const Move currentMove, const size_t moveNum)
{
    println(cout,
        "info currmove {} currmovenumber {}",
        to_uci(currentMove), moveNum);

    cout.flush();
}

void refutation_info(
    const Move move, const MoveSpan refutation)
{
    std::string refLineString;

    for (const auto lineMove : refutation)
        refLineString.append(std::format("{} ", to_uci(lineMove)));

    if (not refLineString.empty())
        refLineString.pop_back(); // remove final space

    println(cout,
        "info refutation {}{}",
        to_uci(move), refLineString);

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

    // [cp <x>]|[mate <x>] [lowerbound|upperbound]
    [[nodiscard]] auto score_string(const Score& score) -> string
    {
        auto string = std::visit(
            util::Visitor {
                [](const Score::Centipawns& centipawns) { return std::format("cp {}", centipawns.value); },
                [](const Score::MateIn& mate) { return std::format("mate {}", mate.moves()); } },
            score.value);

        assert(not(score.lowerBound and score.upperBound));

        if (score.lowerBound)
            string.append(" lowerbound");
        else if (score.upperBound)
            string.append(" upperbound");

        return string;
    }

    // if not empty, begins with a space:
    // * ""
    // * " pv <move...>"
    [[nodiscard]] auto pv_string(const MoveSpan pv) -> string
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

    // if not empty, begins with a space:
    // * ""
    // * " multipv <n>"
    [[nodiscard]] auto multipv_string(
        const std::optional<size_t> multiPV) -> string
    {
        return multiPV
            .transform([](const size_t lineNum) {
                return std::format(" multipv {}", lineNum);
            })
            .value_or(string { });
    }

    // if not empty, begins with a space:
    // * ""
    // * " string <text>"
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
        "info depth {} seldepth {}{} score {} time {} hashfull {} nodes {} nps {} tbhits {}{}{}",
        info.depth, info.selDepth,
        multipv_string(info.multiPV),
        score_string(info.score),
        info.time.count(), info.hashfull, info.nodes,
        info.get_nps(),
        info.tbHits,
        pv_string(info.pv),
        get_extra_info_string(info.extraInformation));

    cout.flush();
}

} // namespace chess::uci::printing
