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
    This file defines the continuation history table.
    @ingroup benbot_data_structures
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint>
#include <libbenbot/data-structures/HistoryGravity.hpp>
#include <libchess/moves/Move.hpp>
#include <utility>

namespace ben_bot {

using chess::moves::Move;
using std::size_t;

/** This structure stores continuation history scores for quiet moves.

    Each score is indexed by an earlier move on the search path and the current
    move, each identified by its moving piece (including its color) and its
    to-square. Because the color is part of the index, the same table can be
    used for the opponent's last move (1 ply back) and for our own last move
    (2 plies back), since those are always played by different sides.

    Updates work the same way as in the main history table. A null previous
    move (from a null move, or from the root & the sentinel entries before it
    in the search stack) has no entry: its score is always 0, and updates
    through it are ignored.

    This table is about 1.1 MiB, so it should be allocated on the heap.

    @ingroup benbot_data_structures
    @see HistoryTable
 */
struct ContinuationHistory final {
    /** The maximum absolute value of any score in the table. */
    static constexpr auto MAX_SCORE { 16384 };

    /** Clears all the stored scores. */
    void clear() noexcept { scores = { }; }

    /** Halves all the stored scores.
        Unlike the main history, the search doesn't call this at the start of
        each search: the table has far more entries, so each one gets few
        updates, and halving it every move would throw most of them away.
     */
    void age() noexcept
    {
        for (auto& row : scores)
            std::ranges::transform(row, row.begin(), [](const std::int16_t score) {
                return static_cast<std::int16_t>(score / 2);
            });
    }

    /** Applies a bonus (or penalty, if negative) to the score of ``move``
        when it's played after ``prevMove``. This does nothing if ``prevMove``
        is null.

        This uses the "history gravity" formula, which keeps
        scores within ``[-MAX_SCORE, MAX_SCORE]``.

        @param prevMoverIsWhite Whether ``prevMove`` was played by white.
        @param prevMove The earlier move on the search path.
        @param moverIsWhite Whether ``move`` is played by white.
        @param move The current move.
        @param bonus The bonus to apply.
     */
    void update(
        const bool prevMoverIsWhite, const Move prevMove,
        const bool moverIsWhite, const Move move,
        const int bonus) noexcept
    {
        if (prevMove.is_null())
            return;

        apply_history_gravity(
            scores[piece_to_index(prevMoverIsWhite, prevMove)][piece_to_index(moverIsWhite, move)],
            bonus, MAX_SCORE);
    }

    /** Returns the score of ``move`` when it's played after ``prevMove``.
        Returns 0 if ``prevMove`` is null.
     */
    [[nodiscard]] auto get(
        const bool prevMoverIsWhite, const Move prevMove,
        const bool moverIsWhite, const Move move) const noexcept -> int
    {
        if (prevMove.is_null())
            return 0;

        return scores[piece_to_index(prevMoverIsWhite, prevMove)][piece_to_index(moverIsWhite, move)];
    }

private:
    static constexpr auto NUM_PIECE_TYPES { 6uz };

    // colored piece type & to-square
    static constexpr auto NUM_PIECE_TO { 2uz * NUM_PIECE_TYPES * 64uz };

    // white pieces are 0-5 & black pieces are 6-11. For castling the moving
    // piece is the king, and for promotions it's the pawn
    [[nodiscard]] static constexpr auto piece_to_index(const bool isWhite, const Move move) noexcept -> size_t
    {
        const auto coloredPiece = (isWhite ? 0uz : NUM_PIECE_TYPES)
                                + static_cast<size_t>(std::to_underlying(move.piece()));

        return (coloredPiece * 64uz) + static_cast<size_t>(move.to().index());
    }

    using Row = std::array<std::int16_t, NUM_PIECE_TO>;

    std::array<Row, NUM_PIECE_TO> scores { };
};

} // namespace ben_bot
