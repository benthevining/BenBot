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
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <span>
#include <stdexcept>
#include <vector>

namespace chess::search {

using std::size_t;
using Eval = eval::Value;

namespace {

    void order_moves_for_search(std::span<Move> moves)
    {
    }

    [[nodiscard]] Eval quiescence(
        Eval alpha, const Eval beta,
        const Position& currentPosition)
    {
        assert(beta > alpha);

        auto evaluation = eval::evaluate(currentPosition);

        return evaluation;

        if (evaluation >= beta)
            return beta;

        alpha = std::max(alpha, evaluation);

        auto moves = moves::generate<true>(currentPosition); // captures only

        if (moves.empty()) {
            if (currentPosition.is_check())
                return eval::MIN; // checkmate

            return 0.; // stalemate
        }

        order_moves_for_search(moves);

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

        // if (moves.empty()) {
        //     if (currentPosition.is_check())
        //         return eval::MIN; // checkmate
        //
        //     return 0.; // stalemate
        // }

        order_moves_for_search(moves);

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

Move find_best_move(const Position& position)
{
    auto moves = moves::generate(position);

    if (moves.empty()) {
        throw std::invalid_argument {
            std::format(
                "No legal moves in position {}",
                notation::to_fen(position))
        };
    }

    order_moves_for_search(moves);

    const auto depth = 4uz;

    Move best {};

    auto alpha = eval::MIN;
    auto beta  = eval::MAX;

    for (const auto& move : moves) {
        const auto newPosition = game::after_move(position, move);

        const auto score = -alpha_beta(-beta, -alpha, newPosition, depth);

        if (score > alpha) {
            best  = move;
            alpha = score;
        }
    }

    assert(best != Move {});

    return best;
}

} // namespace chess::search
