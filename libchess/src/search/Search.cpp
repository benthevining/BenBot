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
#include <libchess/eval/Evaluation.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/search/Search.hpp>
#include <ranges>
#include <vector>

namespace chess::search {

using std::size_t;
using Eval = eval::Value;

namespace {

    [[nodiscard]] Eval quiescence(
        Eval alpha, const Eval beta,
        const Position& currentPosition)
    {
        assert(beta > alpha);

        auto evaluation = eval::evaluate(currentPosition);

        if (evaluation >= beta)
            return beta;

        alpha = std::max(alpha, evaluation);

        const auto moves = moves::generate<true>(currentPosition); // captures only

        // TODO: order moves for searching

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

        if (depth == 0uz)
            return quiescence(alpha, beta, currentPosition);

        const auto moves = moves::generate(currentPosition);

        if (moves.empty()) {
            if (currentPosition.is_check())
                return eval::MIN; // checkmate

            return 0.; // stalemate
        }

        // TODO: order moves for searching

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto evaluation = -alpha_beta(
                -beta, -alpha, newPosition, depth - 1uz);

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
    const auto moves = moves::generate(position);

    const auto scores = moves
                      | std::views::transform(
                          [position](const Move& move) {
                              return alpha_beta(
                                  eval::MIN, eval::MAX,
                                  game::after_move(position, move),
                                  4uz);
                          })
                      | std::ranges::to<std::vector>();

    const auto maxScore = std::ranges::max_element(scores);

    const auto maxScoreIdx = std::ranges::distance(scores.begin(), maxScore);

    return moves.at(maxScoreIdx);
}

} // namespace chess::search
