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
#include <cassert>
#include <libbenbot/data-structures/HistoryGravity.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <utility>

namespace ben_bot {

using chess::moves::Move;

/** This structure stores history heuristic scores for quiet moves and captures.
    Quiet move scores are indexed by side to move and the move's from & to squares.
    Capture scores are indexed by side to move, the moving piece, the to square,
    and the captured piece type.
    Moves that cause beta cutoffs get a bonus, and the moves of the same kind
    that were searched before them get a penalty.

    @ingroup benbot_data_structures
 */
struct HistoryTable final {
    using PieceType = chess::pieces::Type;

    /** The maximum absolute value of any score in the table. */
    static constexpr auto MAX_SCORE { 16384 };

    /** Clears all the stored scores. */
    void clear() noexcept
    {
        scores        = { };
        captureScores = { };
    }

    /** Halves all the stored scores.
        This is called at the start of each search, so that scores
        from previous searches still count, but less than new ones.
     */
    void age() noexcept
    {
        for (auto& color : scores)
            for (auto& from : color)
                std::ranges::transform(from, from.begin(), [](const int score) { return score / 2; });

        for (auto& color : captureScores)
            for (auto& piece : color)
                for (auto& to : piece)
                    std::ranges::transform(to, to.begin(), [](const int score) { return score / 2; });
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

    /** Applies a bonus (or penalty, if negative) to the given capture's score.
        For an en passant capture, ``captured`` is a pawn.
     */
    void update_capture(
        const bool whiteToMove, const Move move, const PieceType captured, const int bonus) noexcept
    {
        apply_history_gravity(get_capture_ref(whiteToMove, move, captured), bonus, MAX_SCORE);
    }

    /** Returns the score for the given capture. */
    [[nodiscard]] auto get_capture(
        const bool whiteToMove, const Move move, const PieceType captured) const noexcept -> int
    {
        assert(captured != PieceType::King);

        return captureScores[whiteToMove ? 1uz : 0uz][std::to_underlying(move.piece())][move.to().index()][std::to_underlying(captured)];
    }

private:
    [[nodiscard]] auto get_capture_ref(
        const bool whiteToMove, const Move move, const PieceType captured) noexcept -> int&
    {
        assert(captured != PieceType::King);

        return captureScores[whiteToMove ? 1uz : 0uz][std::to_underlying(move.piece())][move.to().index()][std::to_underlying(captured)];
    }

    [[nodiscard]] auto get_ref(const bool whiteToMove, const Move move) noexcept -> int&
    {
        return scores[whiteToMove ? 1uz : 0uz][move.from().index()][move.to().index()];
    }

    using Table = std::array<std::array<int, 64uz>, 64uz>;

    std::array<Table, 2uz> scores { };

    // indexed by [moving piece][to square][captured piece], where the captured piece is never a king
    using CaptureTable = std::array<std::array<std::array<int, 5uz>, 64uz>, 6uz>;

    std::array<CaptureTable, 2uz> captureScores { };
};

} // namespace ben_bot
