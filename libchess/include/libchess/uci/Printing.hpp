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
    This file provides some utility functions for printing UCI-style output.
    @ingroup uci
 */

#pragma once

#include <format>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/UCI.hpp>
#include <optional>
#include <print>
#include <string>
#include <string_view>

/** This namespace contains utility functions for printing UCI-style output.
    @ingroup uci
 */
namespace chess::uci::printing {

using moves::Move;

/** Prints a UCI-formatted information string to standard output.
    This function should be used for any informational or debug output that
    an engine wants to print.

    @ingroup uci
 */
inline void info_string(const std::string_view info)
{
    std::println("info string {}", info);
}

/** Prints a UCI-formatted best move string, and optionally a ponder move.

    @ingroup uci
 */
inline void best_move(
    const Move bestMove, const std::optional<Move> ponderMove)
{
    using notation::to_uci;

    std::println(
        "bestmove {}{}",
        to_uci(bestMove),
        ponderMove.has_value()
            ? std::format(" ponder {}", to_uci(*ponderMove))
            : std::string {});
}

} // namespace chess::uci::printing
