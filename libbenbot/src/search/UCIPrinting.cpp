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
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <iostream>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Search.hpp>
#include <libchess/notation/UCI.hpp>
#include <print>
#include <string>

namespace chess::search {

namespace {

    using std::size_t;

    [[nodiscard]] std::string get_score_string(const ben_bot::eval::Score score)
    {
        if (! score.is_mate()) {
            // NB. we pass score.value directly here instead of going through
            // Score's formatter because that extra indirection appears to cost
            // enough time to observably cost some Elo
            return std::format("cp {}", score.value);
        }

        auto plyToMate = score.ply_to_mate();

        if (plyToMate > 0uz)
            ++plyToMate;

        // plies -> moves
        const auto mateIn = plyToMate / 2uz;

        auto mateVal = static_cast<int>(mateIn);

        if (score < 0)
            mateVal *= -1;

        return std::format("mate {}", mateVal);
    }

    [[nodiscard]] std::string get_tt_hits_string(
        const size_t nodesSearched, const size_t ttHits)
    {
        if (nodesSearched == 0uz)
            return {};

        const auto ttHitPcnt = static_cast<double>(ttHits) / static_cast<double>(nodesSearched);

        return std::format(
            " string TT hits {} ({}%)",
            ttHits, ttHitPcnt * 100.);
    }

    [[nodiscard]] size_t get_nodes_per_second(const Callbacks::Result& res)
    {
        const auto seconds = static_cast<double>(res.duration.count()) * 0.001;

        assert(seconds > 0.);

        const auto nps = static_cast<double>(res.nodesSearched) / seconds;

        return static_cast<size_t>(std::round(nps));
    }

    template <bool PrintBestMove>
    void print_uci_info(const Callbacks::Result& res)
    {
        std::println(
            "info depth {} score {} time {} nodes {} nps {}{}",
            res.depth, get_score_string(res.score), res.duration.count(),
            res.nodesSearched, get_nodes_per_second(res),
            get_tt_hits_string(res.nodesSearched, res.transpositionTableHits));

        if constexpr (PrintBestMove) {
            std::println("bestmove {}", notation::to_uci(res.bestMove));

            // Because these callbacks are executed on the searcher background thread,
            // without this flush here, the output may not actually be written when we
            // expect, leading to timeouts or GUIs thinking we've hung/disconnected.
            // Because the best move is always printed last after all info output, we
            // can do the flush only in this branch.
            std::cout.flush();
        }
    }

    void on_book_hit([[maybe_unused]] const Move& move)
    {
        std::println("info string Opening book hit!");
    }

} // namespace

Callbacks Callbacks::make_uci_handler()
{
    return {
        .onSearchComplete = [](const Result& res) { print_uci_info<true>(res); },
        .onIteration = [](const Result& res) { print_uci_info<false>(res); },
        .onOpeningBookHit = [](const Move& move) { on_book_hit(move); }
    };
}

} // namespace chess::search
