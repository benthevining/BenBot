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

    [[nodiscard]] int quiescence(
        int alpha, const int beta,
        const Position& currentPosition)
    {
        // assert(beta > alpha);

        auto evaluation = eval::evaluate(currentPosition);

        if (evaluation >= beta)
            return beta;

        alpha = std::max(alpha, evaluation);

        auto moves = moves::generate<true>(currentPosition); // captures only

        detail::order_moves_for_search(currentPosition, moves);

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            const auto newPosition = game::after_move(currentPosition, move);

            evaluation = -quiescence(-beta, -alpha, newPosition);

            if (evaluation >= beta)
                return beta;

            alpha = std::max(alpha, evaluation);
        }

        return alpha;
    }

    [[nodiscard]] int alpha_beta(
        int alpha, const int beta,
        const Position& currentPosition,
        const size_t    depth)
    {
        // assert(beta > alpha);

        auto moves = moves::generate(currentPosition);

        detail::order_moves_for_search(currentPosition, moves);

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto evaluation = depth > 1uz
                                      ? -alpha_beta(-beta, -alpha, newPosition, depth - 1uz)
                                      : -quiescence(-beta, -alpha, newPosition);

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

    static constexpr auto beta = eval::MATE * 2;

    auto alpha = -beta;

    for (const auto& move : moves) {
        const auto newPosition = game::after_move(position, move);

        const auto score = -alpha_beta(-beta, -alpha, newPosition, searchDepth);

        if (score > alpha) {
            bestMove = move;
            alpha    = score;
        }
    }

    return bestMove.value();
}

} // namespace chess::search
