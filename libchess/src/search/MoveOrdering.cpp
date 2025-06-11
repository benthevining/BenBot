/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include "MoveOrdering.hpp" // NOLINT(build/include_subdir)
#include <algorithm>
#include <libchess/board/Bitboard.hpp>
#include <libchess/eval/Material.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <limits>
#include <span>

namespace chess::search::detail {

namespace {

    using board::Bitboard;
    using pieces::Color;
    using PieceType = pieces::Type;

    [[nodiscard, gnu::const]] Bitboard get_opponent_pawn_attacks(const Position& position) noexcept
    {
        using moves::patterns::pawn_attacks;

        if (position.sideToMove == Color::White)
            return pawn_attacks<Color::Black>(position.their_pieces().pawns);

        return pawn_attacks<Color::White>(position.their_pieces().pawns);
    }

    // higher scored moves will be searched first
    [[nodiscard, gnu::const]] int move_ordering_score(
        const Position& currentPosition, const Move& move,
        const TranspositionTable& transTable,
        const Bitboard            opponentPawnAttacks)
    {
        namespace piece_values = eval::piece_values;

        static constexpr auto PV_NODE_BONUS { 1500 };        // cppcheck-suppress variableScope
        static constexpr auto CAPTURE_MULTIPLIER { 10 };     // cppcheck-suppress variableScope
        static constexpr auto PROMOTION_MULTIPLIER { 15 };   // cppcheck-suppress variableScope
        static constexpr auto CASTLING_BONUS { 30 };         // cppcheck-suppress variableScope
        static constexpr auto PAWN_CONTROLS_PENALTY { 350 }; // cppcheck-suppress variableScope

        const auto& theirPieces = currentPosition.their_pieces();

        auto score { 0 };

        if (const auto* currPosRecord = transTable.find(currentPosition))
            if (currPosRecord->bestMove.has_value() && currPosRecord->bestMove == move)
                return std::numeric_limits<int>::max(); // arbitrarily large score to ensure this move is ordered first

        // look up stored record of resulting position after making move
        if (const auto* record = transTable.find(game::after_move(currentPosition, move))) {
            if (record->evalType == TranspositionTable::Record::EvalType::Exact)
                score += PV_NODE_BONUS;

            score += record->eval;
        }

        if (const auto capturedType = theirPieces.get_piece_on(move.to)) {
            // NB. checking for captures this way prevents en passant from entering this branch

            // we want to prioritize searching moves that capture valuable pieces with less valuable pieces
            score += CAPTURE_MULTIPLIER
                   * (piece_values::get(*capturedType) - piece_values::get(move.piece));
        }

        if (move.is_promotion()) {
            [[unlikely]];
            score += PROMOTION_MULTIPLIER * piece_values::get(*move.promotedType);
        } else if (move.piece != PieceType::Pawn) {
            if (move.is_castling()) {
                [[unlikely]];
                score += CASTLING_BONUS;
            } else if (opponentPawnAttacks.test(move.to)) {
                // Penalize moving piece to a square attacked by opponent pawn
                score -= PAWN_CONTROLS_PENALTY;
            }
        }

        return score;
    }

} // namespace

void order_moves_for_search(
    const Position&           currentPosition,
    const std::span<Move>     moves,
    const TranspositionTable& transTable)
{
    std::ranges::sort(
        moves,
        [&currentPosition, &transTable,
            opponentPawnAttacks = get_opponent_pawn_attacks(currentPosition)](const Move& first, const Move& second) {
            return move_ordering_score(currentPosition, first, transTable, opponentPawnAttacks)
                 > move_ordering_score(currentPosition, second, transTable, opponentPawnAttacks);
        });
}

} // namespace chess::search::detail
