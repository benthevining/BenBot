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
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Material.hpp>
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

namespace ben_bot::search::detail {

namespace {

    namespace piece_values = eval::piece_values;

    using chess::board::Bitboard;
    using chess::pieces::Color;
    using PieceType = chess::pieces::Type;

    [[nodiscard, gnu::const]] auto get_opponent_pawn_attacks(const Position& position) noexcept
        -> Bitboard
    {
        using chess::moves::patterns::pawn_attacks;

        if (position.is_white_to_move())
            return pawn_attacks<Color::Black>(position.their_pieces().pawns);

        return pawn_attacks<Color::White>(position.their_pieces().pawns);
    }

    // higher scored moves will be searched first
    [[nodiscard, gnu::const]] auto move_ordering_score(
        const Position& currentPosition, const Move move,
        const TranspositionTable& transTable,
        const Bitboard            opponentPawnAttacks,
        const std::optional<Move> bestMove,
        std::span<const Move>     killerMoves)
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

        // check if this move was recorded as the best move in this position
        if (bestMove.has_value() and *bestMove == move)
            return std::numeric_limits<int>::max(); // arbitrarily large score to ensure this move is ordered first

        auto score { 0 };

        if (const auto capturedType = currentPosition.their_pieces().get_piece_on(move.to())) {
            // we want to prioritize searching moves that capture valuable pieces with less valuable pieces
            score += CAPTURE_MULTIPLIER
                   * (piece_values::get(*capturedType) - piece_values::get(move.piece()));
        } else if (std::ranges::contains(killerMoves, move)) {
            score += KILLER_MOVE_BONUS;
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
                case Alpha: break;
                default   : break;
            }
        }

        if (posAfterMove.is_check())
            score += CHECK_BONUS;

        return score;
    }

} // namespace

void order_moves_for_search(
    const Position&           currentPosition,
    const std::span<Move>     moves,
    const TranspositionTable& transTable,
    std::span<const Move>     killerMoves)
{
    const auto bestMove = transTable
                              .find(currentPosition)
                              .and_then(&TTData::bestMove);

    std::ranges::sort(
        moves,
        [&currentPosition, &transTable, bestMove, killerMoves,
            opponentPawnAttacks = get_opponent_pawn_attacks(currentPosition)](const Move first, const Move second) {
            return move_ordering_score(currentPosition, first, transTable, opponentPawnAttacks, bestMove, killerMoves)
                 > move_ordering_score(currentPosition, second, transTable, opponentPawnAttacks, bestMove, killerMoves);
        });
}

namespace {
    // MVV-LVA scoring
    [[nodiscard, gnu::const]] auto q_search_move_order_score(
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
    std::ranges::sort(
        moves,
        [&currentPosition](const Move first, const Move second) {
            return q_search_move_order_score(currentPosition, first)
                 > q_search_move_order_score(currentPosition, second);
        });
}

} // namespace ben_bot::search::detail
