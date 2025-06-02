/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/eval/Evaluation.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/search/Search.hpp>
#include <limits>

namespace chess::search {

using std::size_t;
using Eval = eval::Value;

namespace {

    Eval alpha_beta(
        Eval alpha, const Eval beta,
        const Position& currentPosition,
        const size_t    depth,
        Move&           bestMoveThisIteration)
    {
        if (alpha >= beta)
            return alpha;

        if (depth == 0uz) {
            // TODO: quiescence search
            return eval::evaluate(currentPosition);
        }

        const auto moves = moves::generate(currentPosition);

        if (moves.empty()) {
            if (currentPosition.is_check())
                return std::numeric_limits<Eval>::min(); // checkmate

            return 0.; // stalemate
        }

        // TODO: order moves for searching

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto score = -alpha_beta(
                -beta, -alpha, newPosition, depth - 1uz, bestMoveThisIteration);

            if (score >= beta)
                return beta;

            // found a new best move in this position
            if (score > alpha) {
                alpha                 = score;
                bestMoveThisIteration = move;
            }
        }

        return alpha;
    }

} // namespace

Move find_best_move(const Position& position)
{
    Move bestMove {};

    alpha_beta(
        std::numeric_limits<Eval>::min(),
        std::numeric_limits<Eval>::max(),
        position, 3uz,
        bestMove);

    return bestMove;
}

} // namespace chess::search
