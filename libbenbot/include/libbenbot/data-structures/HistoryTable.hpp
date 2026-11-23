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
    This file defines the history heuristic table.
    @ingroup benbot_data_structures
 */

#pragma once

#include <algorithm>
#include <array>
#include <libbenbot/data-structures/HistoryGravity.hpp>
#include <libchess/moves/Move.hpp>

namespace ben_bot {

using chess::moves::Move;

/** This structure stores history heuristic scores for quiet moves.
    Scores are indexed by side to move and the move's from & to squares.
    Moves that cause beta cutoffs get a bonus, and the quiet moves that
    were searched before them get a penalty.

    @ingroup benbot_data_structures
 */
struct HistoryTable final {
    /** The maximum absolute value of any score in the table. */
    static constexpr auto MAX_SCORE { 16384 };

    /** Clears all the stored scores. */
    void clear() noexcept { scores = { }; }

    /** Halves all the stored scores.
        This is called at the start of each search, so that scores
        from previous searches still count, but less than new ones.
     */
    void age() noexcept
    {
        for (auto& color : scores)
            for (auto& from : color)
                std::ranges::transform(from, from.begin(), [](const int score) { return score / 2; });
    }

    /** Applies a bonus (or penalty, if negative) to the given move's score.
        This uses the "history gravity" formula, which keeps
        scores within ``[-MAX_SCORE, MAX_SCORE]``.
     */
    void update(const bool whiteToMove, const Move move, const int bonus) noexcept
    {
        apply_history_gravity(get_ref(whiteToMove, move), bonus, MAX_SCORE);
    }

    /** Returns the score for the given move. */
    [[nodiscard]] auto get(const bool whiteToMove, const Move move) const noexcept -> int
    {
        return scores[whiteToMove ? 1uz : 0uz][move.from().index()][move.to().index()];
    }

private:
    [[nodiscard]] auto get_ref(const bool whiteToMove, const Move move) noexcept -> int&
    {
        return scores[whiteToMove ? 1uz : 0uz][move.from().index()][move.to().index()];
    }

    using Table = std::array<std::array<int, 64uz>, 64uz>;

    std::array<Table, 2uz> scores { };
};

} // namespace ben_bot
