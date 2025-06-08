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
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <optional>
#include <stdexcept>

namespace chess::search {

using std::size_t;

namespace {

    constexpr auto EVAL_MAX = eval::MATE * 2;

    [[nodiscard]] int quiescence(
        int alpha, const int beta,
        const Position& currentPosition,
        const size_t    plyFromRoot)
    {
        // assert(beta > alpha);

        if (currentPosition.is_draw())
            return eval::DRAW;

        auto evaluation = eval::evaluate(currentPosition);

        if (evaluation >= beta)
            return beta;

        alpha = std::max(alpha, evaluation);

        auto moves = moves::generate<true>(currentPosition); // captures only

        if (moves.empty() && currentPosition.is_check()) {
            // checkmate
            return (EVAL_MAX - static_cast<int>(plyFromRoot)) * -1;
        }

        detail::order_moves_for_search(currentPosition, moves);

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            const auto newPosition = game::after_move(currentPosition, move);

            evaluation = -quiescence(-beta, -alpha, newPosition, plyFromRoot + 1uz);

            if (evaluation >= beta)
                return beta;

            alpha = std::max(alpha, evaluation);
        }

        return alpha;
    }

    [[nodiscard]] int alpha_beta(
        int alpha, const int beta,
        const Position& currentPosition,
        const size_t    depth,
        const size_t    plyFromRoot)
    {
        // assert(beta > alpha);

        if (currentPosition.is_draw())
            return eval::DRAW;

        auto moves = moves::generate(currentPosition);

        if (moves.empty() && currentPosition.is_check()) {
            // checkmate
            return (EVAL_MAX - static_cast<int>(plyFromRoot)) * -1;
        }

        detail::order_moves_for_search(currentPosition, moves);

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto evaluation = depth > 1uz
                                      ? -alpha_beta(-beta, -alpha, newPosition, depth - 1uz, plyFromRoot + 1uz)
                                      : -quiescence(-beta, -alpha, newPosition, plyFromRoot + 1uz);

            if (evaluation >= beta)
                return beta;

            alpha = std::max(alpha, evaluation);
        }

        return alpha;
    }

} // namespace

Move find_best_move(
    const Position& position, const size_t searchDepth)
{
    auto moves = moves::generate(position);

    if (moves.empty()) {
        throw std::invalid_argument {
            std::format(
                "No legal moves in position {}",
                notation::to_fen(position))
        };
    }

    detail::order_moves_for_search(position, moves);

    std::optional<Move> bestMove;

    static constexpr auto beta = EVAL_MAX;

    auto alpha = -EVAL_MAX;

    for (const auto& move : moves) {
        const auto newPosition = game::after_move(position, move);

        const auto score = -alpha_beta(-beta, -alpha, newPosition, searchDepth, 1uz);

        if (score > alpha) {
            bestMove = move;
            alpha    = score;
        }
    }

    return bestMove.value();
}

} // namespace chess::search
