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

#include "search/MoveOrdering.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <libbenbot/data-structures/HistoryTable.hpp>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/SEE.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <limits>
#include <optional>
#include <span>
#include <utility>

namespace ben_bot::search::detail {

namespace {

    namespace piece_values = eval::piece_values;

    using chess::board::Bitboard;
    using chess::pieces::Color;
    using PieceType = chess::pieces::Type;

    [[nodiscard, gnu::pure]] auto get_opponent_pawn_attacks(const Position& position) noexcept
        -> Bitboard
    {
        using chess::moves::patterns::pawn_attacks;

        if (position.is_white_to_move())
            return pawn_attacks<Color::Black>(position.their_pieces().pawns);

        return pawn_attacks<Color::White>(position.their_pieces().pawns);
    }

    // higher scored moves will be searched first
    [[nodiscard, gnu::pure]] auto move_ordering_score(
        const Position& currentPosition, const Move move,
        const TranspositionTable&   transTable,
        const Bitboard              opponentPawnAttacks,
        const std::optional<Move>   bestMove,
        const std::span<const Move> killerMoves,
        const HistoryTable&         history)
        -> int
    {
        static constexpr auto PV_NODE_BONUS { 15000 };       // cppcheck-suppress variableScope
        static constexpr auto CUT_NODE_PENALTY { 15000 };    // cppcheck-suppress variableScope
        static constexpr auto CAPTURE_MULTIPLIER { 10000 };  // cppcheck-suppress variableScope
        static constexpr auto KILLER_MOVE_BONUS { 10000 };   // cppcheck-suppress variableScope
        static constexpr auto PROMOTION_MULTIPLIER { 15 };   // cppcheck-suppress variableScope
        static constexpr auto CASTLING_BONUS { 30 };         // cppcheck-suppress variableScope
        static constexpr auto PAWN_CONTROLS_PENALTY { 350 }; // cppcheck-suppress variableScope
        static constexpr auto CHECK_BONUS { 1250 };          // cppcheck-suppress variableScope
        static constexpr auto HISTORY_DIVISOR { 8 };         // cppcheck-suppress variableScope

        // check if this move was recorded as the best move in this position
        if (bestMove.has_value() and *bestMove == move)
            return std::numeric_limits<int>::max(); // arbitrarily large score to ensure this move is ordered first

        auto score { 0 };

        if (currentPosition.is_capture(move)) {
            // we want to prioritize searching captures that win the most material once
            // all recaptures on the square have been played out
            score += CAPTURE_MULTIPLIER * eval::see(currentPosition, move);
        } else if (std::ranges::contains(killerMoves, move)) {
            score += KILLER_MOVE_BONUS;
        } else if (not move.promoted_type().has_value()) {
            score += history.get(currentPosition.is_white_to_move(), move) / HISTORY_DIVISOR;
        }

        if (const auto prom = move.promoted_type()) {
            score += PROMOTION_MULTIPLIER * piece_values::get(prom.value());
        } else if (move.piece() != PieceType::Pawn) {
            if (move.is_castling()) {
                score += CASTLING_BONUS;
            } else if (opponentPawnAttacks.test(move.to())) {
                // Penalize moving piece to a square attacked by opponent pawn
                score -= PAWN_CONTROLS_PENALTY;
            }
        }

        const auto posAfterMove = after_move(currentPosition, move);

        // look up stored record of resulting position after making move
        if (const auto record = transTable.find(posAfterMove)) {
            switch (record->evalType) {
                using enum EvalType;

                case Exact: score += PV_NODE_BONUS; break;
                case Beta : score -= CUT_NODE_PENALTY; break;
                default   : [[fallthrough]];
                case Alpha: break;
            }
        }

        if (posAfterMove.is_check())
            score += CHECK_BONUS;

        return score;
    }

} // namespace

namespace {
    struct ScoredMove final {
        int         score { 0 };
        std::size_t index { 0uz }; // where the move generator put this move
        Move        move;
    };

    // sorts the moves by score, highest first. std::sort doesn't keep equal elements in their
    // original order, and different standard libraries order them differently, so ties are broken
    // by the order the move generator produced the moves in. That keeps the order (and bench) the
    // same on every platform, without std::stable_sort's allocation.
    void sort_scored_moves(const std::span<ScoredMove> scoredMoves, const std::span<Move> moves)
    {
        std::ranges::sort(
            scoredMoves,
            [](const ScoredMove& first, const ScoredMove& second) {
                if (first.score != second.score)
                    return first.score > second.score;

                return first.index < second.index;
            });

        std::ranges::transform(scoredMoves, moves.begin(), &ScoredMove::move);
    }
} // namespace

void order_moves_for_search(
    const Position&             currentPosition,
    const std::span<Move>       moves,
    const TranspositionTable&   transTable,
    const std::span<const Move> killerMoves,
    const HistoryTable&         history)
{
    const auto bestMove = transTable
                              .find(currentPosition)
                              .and_then(&TTData::bestMove);

    const auto opponentPawnAttacks = get_opponent_pawn_attacks(currentPosition);

    // scoring a move is expensive (it makes the move & probes the TT), so we
    // score each move once up front instead of in the sort's comparison function
    assert(moves.size() <= chess::moves::MAX_MOVES);

    std::array<ScoredMove, chess::moves::MAX_MOVES> scored { };

    const auto scoredMoves = std::span { scored }.first(moves.size());

    for (auto idx = 0uz; idx < moves.size(); ++idx) {
        const auto move = moves[idx];

        scoredMoves[idx] = {
            .score = move_ordering_score(currentPosition, move, transTable, opponentPawnAttacks, bestMove, killerMoves, history),
            .index = idx,
            .move  = move
        };
    }

    sort_scored_moves(scoredMoves, moves);
}

namespace {
    // MVV-LVA scoring
    [[nodiscard, gnu::pure]] auto q_search_move_order_score(
        const Position& currentPosition, const Move move) noexcept -> int
    {
        return currentPosition.their_pieces()
            .get_piece_on(move.to())
            .transform([move](const PieceType capturedType) {
                return piece_values::get(capturedType) - std::to_underlying(move.piece());
            })
            .value_or(0);
    }
} // namespace

void order_moves_for_q_search(
    const Position&       currentPosition,
    const std::span<Move> moves)
{
    assert(moves.size() <= chess::moves::MAX_MOVES);

    std::array<ScoredMove, chess::moves::MAX_MOVES> scored { };

    const auto scoredMoves = std::span { scored }.first(moves.size());

    for (auto idx = 0uz; idx < moves.size(); ++idx) {
        const auto move = moves[idx];

        scoredMoves[idx] = {
            .score = q_search_move_order_score(currentPosition, move),
            .index = idx,
            .move  = move
        };
    }

    sort_scored_moves(scoredMoves, moves);
}

} // namespace ben_bot::search::detail
