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

    [[nodiscard]] Eval alpha_beta(
        Eval alpha, const Eval beta,
        const Position& currentPosition,
        const size_t    depth)
    {
        if (depth == 0uz) {
            // TODO: quiescence search
            return eval::evaluate(currentPosition);
        }

        auto maxValue = std::numeric_limits<Eval>::min();

        for (const auto& move : moves::generate(currentPosition)) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto score = -alpha_beta(
                -beta, -alpha, newPosition, depth - 1uz);

            if (score > maxValue) {
                maxValue = score;
                alpha    = std::max(score, alpha);
            }

            if (score >= beta)
                return score;
        }

        return maxValue;
    }

} // namespace

Move find_best_move(const Position& position)
{
    Move bestMove {};

    auto maxValue = std::numeric_limits<Eval>::min();

    for (const auto& move : moves::generate(position)) {
        const auto newPosition = game::after_move(position, move);

        const auto score = alpha_beta(
            std::numeric_limits<Eval>::min(),
            std::numeric_limits<Eval>::max(),
            newPosition, 3uz);

        if (score > maxValue) {
            bestMove = move;
            maxValue = score;
        }
    }

    return bestMove;
}

} // namespace chess::search
