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
    This file turns PGNs of self-play games into a dataset of labelled positions.
    @ingroup tuner
 */

#pragma once

#include <cstddef>
#include <iosfwd>
#include <libchess/notation/PGN.hpp>
#include <optional>
#include <random>
#include <string_view>
#include <tuner/Dataset.hpp>
#include <vector>

namespace ben_bot::tuner {

/** What a move's PGN comment says, as fastchess writes them
    (``{book}``, or ``{+0.36/14 0.244s, n=123456}`` for an engine move).

    @ingroup tuner
 */
struct MoveComment final {
    /** True if the move came from the opening book. */
    bool isBook { false };

    /** True if the score is a mate score (such as ``+M5``). */
    bool isMate { false };

    /** The search score in centipawns, from the point of view of the side that played the move.
        This is ``nullopt`` for book moves, mate scores, and comments without a score.
     */
    std::optional<int> score;
};

/** Parses a move's PGN comment.
    @ingroup tuner
 */
[[nodiscard]] auto parse_move_comment(std::string_view comment) -> MoveComment;

/** A position sampled from a game.
    @ingroup tuner
 */
struct ExtractedPosition final {
    /** The position, labelled with the game's result. */
    LabelledPosition labelled;

    /** The search score of the move played from this position, in centipawns from White's
        point of view, if its comment had one.
     */
    std::optional<int> score;
};

/** Options for extracting positions from games.
    @ingroup tuner
 */
struct ExtractOptions final {
    /** The most positions to take from each game (0 = all of them). */
    size_t positionsPerGame { 10uz };

    /** The number of plies to skip after the book moves (the moves with a ``{book}`` comment). */
    size_t skipPlies { 8uz };
};

/** Counts of what happened while extracting positions.
    @ingroup tuner
 */
struct ExtractStats final {
    /** The number of games read. */
    size_t games { 0 };

    /** The number of games skipped because they couldn't be parsed or had no result. */
    size_t skippedGames { 0 };

    /** The number of positions skipped because they were book moves, or in the plies after them. */
    size_t openingPositions { 0 };

    /** The number of positions dropped because the side to move is in check. */
    size_t inCheck { 0 };

    /** The number of positions dropped because the move played from them had a mate score. */
    size_t mateScores { 0 };

    /** The number of positions that could have been taken (after the filters above). */
    size_t candidates { 0 };

    /** The number of positions taken. */
    size_t extracted { 0 };
};

/** Samples positions from one game.

    The book moves (counted from the start of the game) and the ``skipPlies`` plies after
    them are skipped, and so are positions where the side to move is in check or where the
    move played has a mate score. Of the positions left, ``positionsPerGame`` are chosen at
    random with ``rng`` (or all of them, if there aren't that many), and returned in the
    order they were played. Each is labelled with the game's result from White's point of
    view. Games with no result give no positions.

    @ingroup tuner
 */
[[nodiscard]] auto extract_positions(
    const chess::notation::GameRecord& game, const ExtractOptions& options,
    std::mt19937_64& rng, ExtractStats& stats)
    -> std::vector<ExtractedPosition>;

/** Reads every game in a PGN stream, samples positions from each with ``extract_positions()``,
    and writes them to ``output`` with ``format_labelled_line()``, one per line.

    The games are read one at a time, so the file is never all in memory. For a given
    input & ``rng`` state, the output is always the same: the positions are chosen with
    ``rng`` directly rather than through a standard distribution, whose results differ
    between standard libraries.

    @ingroup tuner
 */
void extract(
    std::istream& pgn, std::ostream& output, const ExtractOptions& options,
    std::mt19937_64& rng, ExtractStats& stats);

} // namespace ben_bot::tuner
