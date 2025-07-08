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
#include <libbenbot/search/Search.hpp>
#include <libchess/notation/UCI.hpp>
#include <print>
#include <string>

namespace chess::search {

namespace {

    using std::size_t;

    [[nodiscard]] std::string get_score_string(const int score)
    {
        if (! detail::is_mate_score(score))
            return std::format("cp {}", score);

        auto plyToMate = detail::ply_to_mate_from_score(score);

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

    template <bool PrintBestMove>
    void print_uci_info(const Callbacks::Result& res)
    {
        std::println(
            "info depth {} score {} time {} nodes {} nps {}",
            res.depth, get_score_string(res.score), res.duration.count(),
            res.nodesSearched, get_nodes_per_second(res));

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

} // namespace

Callbacks Callbacks::make_uci_handler()
{
    return {
        .onSearchComplete = [](const Result& res) { print_uci_info<true>(res); },
        .onIteration = [](const Result& res) { print_uci_info<false>(res); }
    };
}

} // namespace chess::search
