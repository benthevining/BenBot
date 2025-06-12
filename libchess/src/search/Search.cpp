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
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <optional>
#include <stdexcept>

namespace chess::search {

using std::size_t;

namespace {

    using EvalType = TranspositionTable::Record::EvalType;

    // arbitrary value used as the starting beta value
    // this should be larger than mate, but smaller than
    // the data type's max (to avoid issues with sign flipping)
    constexpr auto EVAL_MAX = eval::MATE * 2;

    // mate scores are based on the distance from the root of the tree
    // to the leaf (mate) node, so that the engine actually goes for mate
    [[nodiscard, gnu::const]] int checkmate_score(const size_t plyFromRoot) noexcept
    {
        // multiply by -1 here because this score
        // is relative to the player who got mated
        return (EVAL_MAX - static_cast<int>(plyFromRoot)) * -1;
    }

    // searches only captures, with no depth limit, to try to
    // improve the stability of the static evaluation function
    [[nodiscard]] int quiescence(
        int alpha, const int beta,
        const Position&     currentPosition,
        const size_t        depth, // this value is the depth parameter that was sent to alpha/beta
        const size_t        plyFromRoot,
        TranspositionTable& transTable)
    {
        assert(beta > alpha);

        // check if this position has been searched before to at
        // least this depth and within these bounds for non-PV nodes
        if (const auto value = transTable.probe_eval(currentPosition, depth, alpha, beta))
            return value.value();

        if (currentPosition.is_draw()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval::DRAW,
                                     .evalType    = EvalType::Exact });

            return eval::DRAW;
        }

        auto evaluation = eval::evaluate(currentPosition);

        if (evaluation >= beta) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = beta,
                                     .evalType    = EvalType::Beta });

            return beta;
        }

        alpha = std::max(alpha, evaluation);

        auto moves = moves::generate<true>(currentPosition); // captures only

        if (moves.empty() && currentPosition.is_check()) {
            const auto eval = checkmate_score(plyFromRoot);

            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval, // TODO: needs scaling/mapping?
                                     .evalType    = EvalType::Exact });

            return eval;
        }

        detail::order_moves_for_search(currentPosition, moves, transTable);

        auto evalType { EvalType::Alpha };

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            const auto newPosition = game::after_move(currentPosition, move);

            evaluation = -quiescence(-beta, -alpha, newPosition, depth + 1uz, plyFromRoot + 1uz, transTable);

            if (evaluation >= beta) {
                transTable.store(
                    currentPosition, { .searchedDepth = depth,
                                         .eval        = beta,
                                         .evalType    = EvalType::Beta });

                return beta;
            }

            if (evaluation > alpha) {
                evalType = EvalType::Exact;
                alpha    = evaluation;
            }
        }

        transTable.store(
            currentPosition, { .searchedDepth = depth,
                                 .eval        = alpha,
                                 .evalType    = evalType });

        return alpha;
    }

    [[nodiscard]] int alpha_beta(
        int alpha, const int beta,
        const Position&     currentPosition,
        const size_t        depth,
        const size_t        plyFromRoot,
        TranspositionTable& transTable)
    {
        assert(beta > alpha);

        // check if this position has been searched before to at
        // least this depth and within these bounds for non-PV nodes
        if (const auto value = transTable.probe_eval(currentPosition, depth, alpha, beta))
            return value.value();

        if (currentPosition.is_draw()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval::DRAW,
                                     .evalType    = EvalType::Exact });

            return eval::DRAW;
        }

        auto moves = moves::generate(currentPosition);

        if (moves.empty() && currentPosition.is_check()) {
            const auto eval = checkmate_score(plyFromRoot);

            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval, // TODO: needs scaling/mapping?
                                     .evalType    = EvalType::Exact });

            return eval;
        }

        detail::order_moves_for_search(currentPosition, moves, transTable);

        auto evalType { EvalType::Alpha };

        std::optional<Move> bestMove;

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            const auto eval = depth > 1uz
                                ? -alpha_beta(-beta, -alpha, newPosition, depth - 1uz, plyFromRoot + 1uz, transTable)
                                : -quiescence(-beta, -alpha, newPosition, depth, plyFromRoot + 1uz, transTable);

            if (eval >= beta) {
                transTable.store(
                    currentPosition, { .searchedDepth = depth,
                                         .eval        = beta,
                                         .evalType    = EvalType::Beta,
                                         .bestMove    = bestMove });

                return beta;
            }

            if (eval > alpha) {
                bestMove = move;
                evalType = EvalType::Exact;
                alpha    = eval;
            }
        }

        transTable.store(
            currentPosition, { .searchedDepth = depth,
                                 .eval        = alpha,
                                 .evalType    = evalType,
                                 .bestMove    = bestMove });

        return alpha;
    }

} // namespace

Move find_best_move(
    const Position& position, TranspositionTable& transTable, const size_t searchDepth)
{
    auto moves = moves::generate(position);

    if (moves.empty()) {
        throw std::invalid_argument {
            std::format(
                "No legal moves in position {}",
                notation::to_fen(position))
        };
    }

    std::optional<Move> bestMove;

    static constexpr auto beta = EVAL_MAX;

    auto alpha = -EVAL_MAX;

    // iterative deepening
    for (auto depth = 1uz; depth <= searchDepth; ++depth) {
        // we can generate the legal moves only once, but we should reorder them each iteration
        // because the move ordering will change based on the evaluations done during the last iteration
        detail::order_moves_for_search(position, moves, transTable);

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(position, move);

            const auto score = -alpha_beta(-beta, -alpha, newPosition, searchDepth, 1uz, transTable);

            if (score > alpha) {
                bestMove = move;
                alpha    = score;
            }
        }
    }

    return bestMove.value();
}

} // namespace chess::search
