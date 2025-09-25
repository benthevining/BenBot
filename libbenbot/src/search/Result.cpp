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

#include <format>
#include <libbenbot/search/Result.hpp>
#include <string>

namespace ben_bot::search {

namespace {

    [[nodiscard]] auto get_extra_stats_string(
        const Result& res, const bool includeDebugInfo) -> std::string
    {
        if (res.nodesSearched == 0uz or not includeDebugInfo)
            return {};

        auto get_pcnt = [totalNodes = static_cast<double>(res.nodesSearched)](const size_t value) {
            return (static_cast<double>(value) / totalNodes) * 100.;
        };

        return std::format(
            "TT hits {} ({:.1f}%) Beta cutoffs {} ({:.1f}%) MDP cutoffs {} ({:.1f}%)",
            res.transpositionTableHits, get_pcnt(res.transpositionTableHits),
            res.betaCutoffs, get_pcnt(res.betaCutoffs),
            res.mdpCutoffs, get_pcnt(res.mdpCutoffs));
    }

} // namespace

auto Result::to_libchess(const bool includeDebugInfo) const -> LibchessResult
{
    return {
        .score            = score.to_libchess(),
        .depth            = depth,
        .time             = duration,
        .nodes            = nodesSearched,
        .pv               = pv,
        .hashfull         = hashfull,
        .tbHits           = 0uz,
        .extraInformation = get_extra_stats_string(*this, includeDebugInfo)
    };
}

} // namespace ben_bot::search
