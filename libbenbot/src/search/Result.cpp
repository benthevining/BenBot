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
#include <format>
#include <libbenbot/search/Result.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Variant.hpp>
#include <string>
#include <variant>

namespace ben_bot::search {

using std::string;

namespace {

    [[nodiscard]] auto get_extra_stats_string(
        const Result& res, const bool includeDebugInfo) -> string
    {
        if (not includeDebugInfo)
            return { };

        if (res.nodesSearched == 0uz) {
            return std::format(
                "TT hits {} Beta cutoffs {} MDP cutoffs {} Static evals {}",
                res.transpositionTableHits, res.betaCutoffs, res.mdpCutoffs, res.staticEvals);
        }

        auto get_pcnt = [totalNodes = static_cast<double>(res.nodesSearched)](const size_t value) {
            return (static_cast<double>(value) / totalNodes) * 100.;
        };

        return std::format(
            "TT hits {} ({:.1f}%) Beta cutoffs {} ({:.1f}%) MDP cutoffs {} ({:.1f}%) Static evals {} ({:.1f}%)",
            res.transpositionTableHits, get_pcnt(res.transpositionTableHits),
            res.betaCutoffs, get_pcnt(res.betaCutoffs),
            res.mdpCutoffs, get_pcnt(res.mdpCutoffs),
            res.staticEvals, get_pcnt(res.staticEvals));
    }

} // namespace

auto Result::to_libchess(const bool includeDebugInfo) const -> LibchessResult
{
    return {
        .score            = score.to_libchess(),
        .depth            = depth,
        .selDepth         = qDepth,
        .time             = duration,
        .nodes            = nodesSearched,
        .multiPV          = std::nullopt,
        .pv               = pv,
        .hashfull         = hashfull,
        .tbHits           = 0uz,
        .extraInformation = get_extra_stats_string(*this, includeDebugInfo)
    };
}

namespace {
    [[nodiscard, gnu::const]] auto to_seconds(const milliseconds millis)
    {
        return duration_cast<std::chrono::seconds>(millis).count();
    }

    using chess::notation::to_uci;

    [[nodiscard]] auto format_pv(const MoveList& pv) -> string
    {
        string result;

        for (const auto move : pv) {
            result.append(to_uci(move));
            result.append(1uz, ' ');
        }

        result.pop_back(); // remove trailing space

        return result;
    }
} // namespace

void Result::fill_standard_epd_operations(
    chess::notation::EPDPosition& position) const
{
    namespace ops = chess::notation::epd_ops;

    auto& operations = position.operations;

    operations[string { ops::AnalysisCountDepth }]   = std::format("{}", depth);
    operations[string { ops::AnalysisCountNodes }]   = std::format("{}", nodesSearched);
    operations[string { ops::AnalysisCountSeconds }] = std::format("{}", to_seconds(duration));
    operations[string { ops::PredictedVariation }]   = format_pv(pv);
    operations[string { ops::BestMove }]             = to_uci(best_move());

    ponder_move().transform([&operations](const Move ponder) {
        operations[string { ops::PredictedMove }] = to_uci(ponder);
        return std::monostate { };
    });

    using LibchessScore = eval::Score::LibchessScore;

    std::visit(
        util::Visitor {
            [&operations](const LibchessScore::Centipawns centi) {
                operations[string { ops::CentipawnEvaluation }] = std::format("{}", centi.value);
            },
            [&operations](const LibchessScore::MateIn mate) {
                operations[string { ops::DirectMate }] = std::format("{}", mate.moves());
            } },
        score.to_libchess().value);
}

} // namespace ben_bot::search
