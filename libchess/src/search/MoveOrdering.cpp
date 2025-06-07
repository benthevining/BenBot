/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include "MoveOrdering.hpp" // NOLINT(build/include_subdir)
#include <algorithm>
#include <cassert>
#include <libchess/board/Bitboard.hpp>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/eval/Material.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <span>

namespace chess::search::detail {

namespace {

    using board::Bitboard;
    using pieces::Color;
    using Eval = eval::Value;

    namespace piece_values = eval::piece_values;

    [[nodiscard, gnu::const]] Bitboard get_opponent_pawn_attacks(const Position& position) noexcept
    {
        if (position.sideToMove == Color::White)
            return moves::patterns::pawn_attacks<Color::Black>(position.their_pieces().pawns);

        return moves::patterns::pawn_attacks<Color::White>(position.their_pieces().pawns);
    }

    // higher scored moves will be searched first
    [[nodiscard, gnu::const]] Eval move_ordering_score(
        const Position& currentPosition, const Move& move, const Bitboard opponentPawnAttacks) noexcept
    {
        static constexpr Eval CAPTURE_MULTIPLIER { 10. };
        static constexpr Eval PROMOTION_MULTIPLIER { 7. };
        static constexpr Eval CASTLING_BONUS { 200. };
        static constexpr Eval OPPONENT_PAWN_CONTROLS_PENALTY { 350. };

        const auto& theirPieces = currentPosition.their_pieces();

        Eval score { 0. };

        if (currentPosition.is_capture(move)) {
            if (currentPosition.is_en_passant(move)) {
                [[unlikely]];
                score += CAPTURE_MULTIPLIER;
            } else {
                const auto capturedType = theirPieces.get_piece_on(move.to);

                assert(capturedType.has_value());

                score += CAPTURE_MULTIPLIER
                       * (piece_values::get(*capturedType)
                           - piece_values::get(move.piece));
            }
        }

        if (move.is_promotion()) {
            [[unlikely]];
            score += PROMOTION_MULTIPLIER * piece_values::get(*move.promotedType);
        } else if (move.piece != pieces::Type::Pawn) {
            if (move.is_castling()) {
                [[unlikely]];
                score += CASTLING_BONUS;
            } else {
                // Penalize moving piece to a square attacked by opponent pawn
                if (opponentPawnAttacks.test(move.to))
                    score -= OPPONENT_PAWN_CONTROLS_PENALTY;
            }
        }

        return score;
    }

} // namespace

void order_moves_for_search(
    const Position&       currentPosition,
    const std::span<Move> moves)
{
    std::ranges::sort(
        moves,
        [&currentPosition, opponentPawnAttacks = get_opponent_pawn_attacks(currentPosition)](const Move& first, const Move& second) {
            return move_ordering_score(currentPosition, first, opponentPawnAttacks)
                 > move_ordering_score(currentPosition, second, opponentPawnAttacks);
        });
}

} // namespace chess::search::detail
