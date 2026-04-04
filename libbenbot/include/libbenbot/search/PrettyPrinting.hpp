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

#pragma once

#include <chrono>
#include <libchess/uci/Printing.hpp>
#include <string>

/** This namespace contains utility functions for pretty printing search results.
    @ingroup libbenbot
 */
namespace ben_bot::pretty_print {

/** Pretty prints a duration in the form "10 ms", "1.3 s", "2.5 m", etc.
    @ingroup libbenbot
 */
[[nodiscard]] auto duration(
    std::chrono::milliseconds duration) -> std::string;

using Score = chess::uci::printing::SearchInfo::Score;

/** Pretty prints an evaluation in the form "+1.2", "-3.6", "#3", etc.
    @ingroup libbenbot
 */
[[nodiscard]] auto evaluation(
    Score score) -> std::string;

/** Pretty prints a number of nodes in the form "1k", "1m", etc.
    @ingroup libbenbot
 */
[[nodiscard]] auto nodes(
    size_t num) -> std::string;

/** Pretty prints nodes-per-second in the form "1k/s", "1m/s", etc.
    @ingroup libbenbot
 */
[[nodiscard]] auto nps(
    size_t nodesPerSec) -> std::string;

/** Pretty prints the hashfull permille value as a percentage string.
    @ingroup libbenbot
 */
[[nodiscard]] auto hashfull(
    size_t permille) -> std::string;

/** Pretty prints a search statistic (TT hits, beta cutoffs, etc) as
    a string of the form "1k", "1m", followed by a percentage of the
    totalNodes.
    @ingroup libbenbot
 */
[[nodiscard]] auto search_stat(
    size_t stat, size_t totalNodes) -> std::string;

} // namespace ben_bot::pretty_print
