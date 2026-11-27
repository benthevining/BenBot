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
    This file makes a dataset's positions quiet, with a small deterministic quiescence search.
    @ingroup tuner
 */

#pragma once

#include <cstddef>
#include <iosfwd>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <string>
#include <vector>

namespace ben_bot::tuner {

using chess::game::Position;
using chess::moves::Move;
using std::size_t;

/** A position made quiet by ``resolve()``.
    @ingroup tuner
 */
struct Resolved final {
    /** The position at the end of the principal variation. */
    Position position;

    /** The moves that lead from the original position to ``position``. */
    std::vector<Move> pv;

    /** The quiescence search's score, from the point of view of the original position's side to move. */
    int score { 0 };
};

/** Makes a position quiet by following the principal variation of a quiescence search to its end.

    The search only looks at captures & queen promotions, skips the ones that lose material by
    static exchange evaluation, and stands pat on the current eval. When the side to move is in
    check, it searches every legal move instead, and doesn't stand pat. It has no transposition
    table, history or time limit, so the same position always gives the same result.

    A position that's already quiet is returned unchanged, with an empty PV. The result can still
    be in check if the PV ends in checkmate.

    @ingroup tuner
 */
[[nodiscard]] auto resolve(const Position& position) -> Resolved;

/** Counts of what happened while resolving a dataset.
    @ingroup tuner
 */
struct ResolveStats final {
    /** The number of lines read. */
    size_t lines { 0 };

    /** The number of empty or comment (``#``) lines skipped. */
    size_t skipped { 0 };

    /** The number of lines that couldn't be parsed. */
    size_t parseErrors { 0 };

    /** The first few parse errors. */
    std::vector<std::string> firstErrors;

    /** The number of positions that were already quiet. */
    size_t unchanged { 0 };

    /** The number of positions that were replaced by the end of their PV. */
    size_t changed { 0 };

    /** The number of positions dropped because their PV ended in check (checkmate). */
    size_t inCheck { 0 };
};

/** Reads labelled positions from ``input`` (see ``parse_labelled_line()``), resolves each one with
    ``resolve()``, and writes the resolved positions to ``output`` with ``format_labelled_line()``,
    in the same order. Each keeps its original result, and its search score if the line had one.

    @ingroup tuner
 */
void resolve_dataset(std::istream& input, std::ostream& output, size_t numThreads, ResolveStats& stats);

} // namespace ben_bot::tuner
