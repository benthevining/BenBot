/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <cassert>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/search/Search.hpp>
#include <span>
#include <stdexcept>
#include <vector>

namespace chess::search {

using pieces::Color;
using std::size_t;
using Eval      = eval::Value;
using PieceType = pieces::Type;

namespace {

    // higher scored moves will be searched first
    [[nodiscard, gnu::const]] Eval move_ordering_score(
        const Position& currentPosition, const Move& move) noexcept
    {
        static constexpr Eval CAPTURE_MULTIPLIER { 10. };
        static constexpr Eval PROMOTION_MULTIPLIER { 7. };
        static constexpr Eval CASTLING_BONUS { 200. };
        static constexpr Eval OPPONENT_PAWN_CONTROLS_PENALTY { 350. };

        const auto& theirPieces = currentPosition.sideToMove == Color::White ? currentPosition.blackPieces : currentPosition.whitePieces;

        Eval score { 0. };

        if (currentPosition.is_capture(move)) {
            if (currentPosition.is_en_passant(move)) {
                score += CAPTURE_MULTIPLIER;
            } else {
                const auto capturedType = theirPieces.get_piece_on(move.to);

                assert(capturedType.has_value());

                score += CAPTURE_MULTIPLIER
                           * static_cast<Eval>(pieces::values::get(*capturedType))
                       - static_cast<Eval>(pieces::values::get(move.piece));
            }
        }

        if (move.is_promotion()) {
            score += PROMOTION_MULTIPLIER * static_cast<Eval>(pieces::values::get(*move.promotedType));
        } else if (move.piece != PieceType::Pawn) {
            if (move.is_castling()) {
                score += CASTLING_BONUS;
            } else {
                // Penalize moving piece to a square attacked by opponent pawn
                const auto opponentPawnAttacks = currentPosition.sideToMove == Color::White
                                                   ? moves::patterns::pawn_attacks<Color::Black>(theirPieces.pawns)
                                                   : moves::patterns::pawn_attacks<Color::White>(theirPieces.pawns);

                if (opponentPawnAttacks.test(move.to))
                    score -= OPPONENT_PAWN_CONTROLS_PENALTY;
            }
        }

        return score;
    }

    void order_moves_for_search(
        const Position& currentPosition,
        std::span<Move> moves)
    {
        std::ranges::sort(
            moves,
            [&currentPosition](const Move& first, const Move& second) {
                return move_ordering_score(currentPosition, first)
                     > move_ordering_score(currentPosition, second);
            });
    }

    [[nodiscard]] Eval quiescence(
        Eval alpha, const Eval beta,
        const Position& currentPosition)
    {
        assert(beta > alpha);

        auto evaluation = eval::evaluate(currentPosition);

        if (evaluation >= beta)
            return beta;

        alpha = std::max(alpha, evaluation);

        auto moves = moves::generate<true>(currentPosition); // captures only

        if (moves.empty()) {
            if (currentPosition.is_check())
                return eval::MIN; // checkmate

            return 0.; // stalemate
        }

        order_moves_for_search(currentPosition, moves);

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            evaluation = -quiescence(-beta, -alpha, newPosition);

            if (evaluation >= beta)
                return beta;

            alpha = std::max(alpha, evaluation);
        }

        return alpha;
    }

    [[nodiscard]] Eval alpha_beta(
        Eval alpha, const Eval beta,
        const Position& currentPosition,
        const size_t    depth)
    {
        assert(beta > alpha);

        auto moves = moves::generate(currentPosition);

        if (moves.empty()) {
            if (currentPosition.is_check())
                return eval::MIN; // checkmate

            return 0.; // stalemate
        }

        order_moves_for_search(currentPosition, moves);

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto evaluation = depth > 1uz
                                      ? -alpha_beta(-beta, -alpha, newPosition, depth - 1uz)
                                      : -quiescence(-beta, -alpha, newPosition);

            if (evaluation >= beta)
                return beta; // move was too good, opponent will avoid this position

            // found a new best move in this position
            alpha = std::max(alpha, evaluation);
        }

        return alpha;
    }

} // namespace

Move find_best_move(const Position& position, const size_t searchDepth)
{
    auto moves = moves::generate(position);

    if (moves.empty()) {
        throw std::invalid_argument {
            std::format(
                "No legal moves in position {}",
                notation::to_fen(position))
        };
    }

    order_moves_for_search(position, moves);

    Move best {};

    auto alpha = eval::MIN;
    auto beta  = eval::MAX;

    for (const auto& move : moves) {
        const auto newPosition = game::after_move(position, move);

        const auto score = -alpha_beta(-beta, -alpha, newPosition, searchDepth);

        if (score > alpha) {
            best  = move;
            alpha = score;
        }
    }

    if (best == Move {}) {
        // assert(false);
        return moves.front();
    }

    return best;
}

} // namespace chess::search
