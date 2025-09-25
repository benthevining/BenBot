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

/** @file
    This file defines the search result type.
    @ingroup search
 */

#pragma once

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t;
#include <format>
#include <libbenbot/eval/Score.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/uci/Printing.hpp>
#include <string>

namespace ben_bot::search {

using chess::moves::Move;
using std::chrono::milliseconds;
using std::size_t;

/** The results from a completed search.
    This struct can also represent a still-in-progress search (ie, an update
    on each iterative deepening iteration).
 */
struct Result final {
    /** The total amount of time spent searching to produce this result.
        For depths greater than 1, this value is the duration of the entire
        search, including lower depths of the iterative deepening loop.
     */
    milliseconds duration { 0uz };

    /** The total depth that was searched.
        This value is the highest depth that was completed.
     */
    size_t depth { 0uz };

    /** The evaluation of the position resulting from playing the best move. */
    eval::Score score;

    /** The best move found in the position. */
    Move bestMove;

    /** The total number of nodes visited during this search. For depths greater
        than 1, this value includes nodes visited in shallower depths of the
        iterative deepening loop.
     */
    size_t nodesSearched { 0uz };

    /** The number of nodes for which a saved score from the transposition table
        could be used.
     */
    size_t transpositionTableHits { 0uz };

    /** Number of cutoffs due to alpha-beta. */
    size_t betaCutoffs { 0uz };

    /** Number of cutoffs due to mate distance pruning. */
    size_t mdpCutoffs { 0uz };

    /** The approximate fraction of transposition table entries (permille) that
        were written to during this root search.
     */
    size_t hashfull { 0uz };

    /** Typedef for the libchess type used for printing UCI-formatted output. */
    using LibchessResult = chess::uci::printing::SearchInfo;

    /** Converts this object to the libchess type used for printing UCI-formatted output. */
    [[nodiscard]] LibchessResult to_libchess(bool includeDebugInfo) const;
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */
namespace detail {
    [[nodiscard]] inline auto get_extra_stats_string(
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
} // namespace detail

inline auto Result::to_libchess(const bool includeDebugInfo) const -> LibchessResult
{
    return {
        .score            = score.to_libchess(),
        .depth            = depth,
        .time             = duration,
        .nodes            = nodesSearched,
        .pv               = {},
        .hashfull         = hashfull,
        .tbHits           = 0uz,
        .extraInformation = detail::get_extra_stats_string(*this, includeDebugInfo)
    };
}

} // namespace ben_bot::search
