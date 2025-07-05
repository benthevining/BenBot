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

#include "MoveOrdering.hpp" // NOLINT(build/include_subdir)
#include <algorithm>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/search/TranspositionTable.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <limits>
#include <optional>
#include <span>
#include <utility>

namespace chess::search::detail {

namespace {

    using board::Bitboard;
    using pieces::Color;
    using PieceType = pieces::Type;

    [[nodiscard, gnu::const]] Bitboard get_opponent_pawn_attacks(const Position& position) noexcept
    {
        using moves::patterns::pawn_attacks;

        if (position.is_white_to_move())
            return pawn_attacks<Color::Black>(position.their_pieces().pawns);

        return pawn_attacks<Color::White>(position.their_pieces().pawns);
    }

    // higher scored moves will be searched first
    [[nodiscard, gnu::const]] int move_ordering_score(
        const Position& currentPosition, const Move& move,
        const TranspositionTable& transTable,
        const Bitboard            opponentPawnAttacks,
        const std::optional<Move> bestMove)
    {
        namespace piece_values = eval::piece_values;

        static constexpr auto PV_NODE_BONUS { 15000 };       // cppcheck-suppress variableScope
        static constexpr auto CUT_NODE_PENALTY { -15000 };   // cppcheck-suppress variableScope
        static constexpr auto CAPTURE_MULTIPLIER { 10 };     // cppcheck-suppress variableScope
        static constexpr auto PROMOTION_MULTIPLIER { 15 };   // cppcheck-suppress variableScope
        static constexpr auto CASTLING_BONUS { 30 };         // cppcheck-suppress variableScope
        static constexpr auto PAWN_CONTROLS_PENALTY { 350 }; // cppcheck-suppress variableScope

        // check if this move was recorded as the best move in this position
        if (bestMove.has_value() && *bestMove == move)
            return std::numeric_limits<int>::max(); // arbitrarily large score to ensure this move is ordered first

        // look up stored record of resulting position after making move
        if (const auto* record = transTable.find(after_move(currentPosition, move))) {
            switch (record->evalType) {
                using enum TranspositionTable::Record::EvalType;

                case Exact: return PV_NODE_BONUS;
                case Beta : return CUT_NODE_PENALTY;
                default   : break;
            }
        }

        auto score { 0 };

        if (const auto capturedType = currentPosition.their_pieces().get_piece_on(move.to)) {
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
    std::optional<Move> bestMove;

    // do this lookup only once
    if (const auto* currPosRecord = transTable.find(currentPosition))
        bestMove = currPosRecord->bestMove;

    std::ranges::sort(
        moves,
        [&currentPosition, &transTable, bestMove,
            opponentPawnAttacks = get_opponent_pawn_attacks(currentPosition)](const Move& first, const Move& second) {
            return move_ordering_score(currentPosition, first, transTable, opponentPawnAttacks, bestMove)
                 > move_ordering_score(currentPosition, second, transTable, opponentPawnAttacks, bestMove);
        });
}

// in quiescence search, moves are ordered simply by the value of the captured piece
void order_moves_for_q_search(
    const Position& currentPosition,
    std::span<Move> moves)
{
    const auto& theirPieces = currentPosition.their_pieces();

    auto get_captured_type = [&currentPosition, &theirPieces](const Move& move) {
        if (currentPosition.is_en_passant(move))
            return PieceType::Pawn;

        return theirPieces.get_piece_on(move.to).value();
    };

    std::ranges::sort(
        moves,
        [captured_type = std::move(get_captured_type)](const Move& first, const Move& second) {
            return eval::piece_values::get(captured_type(first))
                 > eval::piece_values::get(captured_type(second));
        });
}

} // namespace chess::search::detail
