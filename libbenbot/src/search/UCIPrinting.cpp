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

#include "libbenbot/search/Search.hpp"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <iostream>
#include <libbenbot/eval/Score.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/UCI.hpp>
#include <optional>
#include <print>
#include <string>

namespace ben_bot::search {

namespace {

    using chess::moves::Move;
    using std::size_t;

    [[nodiscard]] std::string get_score_string(const eval::Score score)
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

    [[nodiscard]] size_t get_nodes_per_second(const Callbacks::Result& res)
    {
        const auto seconds = static_cast<double>(res.duration.count()) * 0.001;

        assert(seconds > 0.);

        const auto nps = static_cast<double>(res.nodesSearched) / seconds;

        return static_cast<size_t>(std::round(nps));
    }

    [[nodiscard]] std::string get_extra_stats_string(const Callbacks::Result& res)
    {
        if (res.nodesSearched == 0uz)
            return {};

        auto get_pcnt = [totalNodes = static_cast<double>(res.nodesSearched)](const size_t value) {
            return (static_cast<double>(value) / totalNodes) * 100.;
        };

        return std::format(
            " string TT hits {} ({}%) Beta cutoffs {} ({}%) MDP cutoffs {} ({}%)",
            res.transpositionTableHits, get_pcnt(res.transpositionTableHits),
            res.betaCutoffs, get_pcnt(res.betaCutoffs),
            res.mdpCutoffs, get_pcnt(res.mdpCutoffs));
    }

    [[nodiscard]] std::string get_ponder_move_string(const std::optional<Move> ponderMove)
    {
        if (! ponderMove.has_value())
            return {};

        return std::format(
            " ponder {}",
            chess::notation::to_uci(*ponderMove));
    }

    template <bool PrintBestMove>
    void print_uci_info(const Callbacks::Result& res)
    {
        std::println(
            "info depth {} score {} time {} nodes {} nps {}{}",
            res.depth, get_score_string(res.score), res.duration.count(),
            res.nodesSearched, get_nodes_per_second(res),
            get_extra_stats_string(res));

        if constexpr (PrintBestMove) {
            std::println("bestmove {}{}",
                chess::notation::to_uci(res.bestMove),
                get_ponder_move_string(res.bestResponse));

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

} // namespace ben_bot::search
