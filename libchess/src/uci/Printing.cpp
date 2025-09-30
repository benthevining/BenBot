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
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <optional>
#include <print>
#include <string>
#include <string_view>

namespace chess::uci::printing {

void info_string(const std::string_view info)
{
    std::println("info string {}", info);
}

using notation::to_uci;

namespace {
    [[nodiscard]] auto ponder_move_string(const std::optional<Move> ponderMove) -> std::string
    {
        return ponderMove
            .transform([](const Move move) { return std::format(" ponder {}", to_uci(move)); })
            .value_or(std::string {});
    }
} // namespace

void best_move(
    const Move bestMove, const std::optional<Move> ponderMove)
{
    std::println(
        "bestmove {}{}",
        to_uci(bestMove), ponder_move_string(ponderMove));
}

namespace {
    [[nodiscard, gnu::const]] auto plies_to_moves(int plies) noexcept -> int
    {
        if (plies > 0)
            ++plies;

        return plies / 2;
    }

    [[nodiscard]] auto base_score_string(const SearchInfo::Score& score) -> std::string
    {
        return score.cp
            .transform([](const int cp) {
                return std::format("cp {}", cp);
            })
            .or_else([&score] {
                return score.mate.transform([](const int pliesToMate) {
                    return std::format("mate {}", plies_to_moves(pliesToMate));
                });
            })
            .value_or(std::string {});
    }

    [[nodiscard]] auto score_string(const SearchInfo::Score& score) -> std::string
    {
        auto string = base_score_string(score);

        if (score.lowerBound)
            string.append(" lowerbound");
        else if (score.upperBound)
            string.append(" upperbound");

        return string;
    }

    [[nodiscard]] auto pv_string(const moves::MoveList& pv) -> std::string
    {
        if (pv.empty()) {
            [[unlikely]];
            return {};
        }

        std::string result { " pv " };

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

        if (seconds <= 0.) {
            [[unlikely]];
            return 0uz;
        }

        const auto nps = static_cast<double>(info.nodes) / seconds;

        return static_cast<size_t>(std::round(nps));
    }

    [[nodiscard]] auto get_extra_info_string(const std::string_view info) -> std::string
    {
        if (info.empty())
            return {};

        return std::format(" string {}", info);
    }
} // namespace

void search_info(const SearchInfo& info)
{
    std::println(
        "info depth {} score {} time {} hashfull {} nodes {} nps {} seldepth {} tbhits {}{}{}",
        info.depth,
        score_string(info.score),
        info.time.count(), info.hashfull, info.nodes,
        get_nodes_per_second(info),
        info.selDepth, info.tbHits,
        pv_string(info.pv),
        get_extra_info_string(info.extraInformation));
}

} // namespace chess::uci::printing
