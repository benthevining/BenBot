/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include "MoveOrdering.hpp" // NOLINT(build/include_subdir)
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <iterator>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <vector>

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
            evaluation = checkmate_score(plyFromRoot);

            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = evaluation, // TODO: needs scaling/mapping?
                                     .evalType    = EvalType::Exact });

            return evaluation;
        }

        detail::order_moves_for_search(currentPosition, moves, transTable);

        auto evalType { EvalType::Alpha };

        // even though we're only searching captures, we can still record
        // the best ones found to help with move ordering in later searches
        std::optional<Move> bestMove;

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            const auto newPosition = game::after_move(currentPosition, move);

            evaluation = -quiescence(
                -beta, -alpha, newPosition, depth + 1uz, plyFromRoot + 1uz, transTable);

            if (evaluation >= beta) {
                transTable.store(
                    currentPosition, { .searchedDepth = depth,
                                         .eval        = beta,
                                         .evalType    = EvalType::Beta,
                                         .bestMove    = bestMove });

                return beta;
            }

            if (evaluation > alpha) {
                bestMove = move;
                evalType = EvalType::Exact;
                alpha    = evaluation;
            }
        }

        transTable.store(
            currentPosition, { .searchedDepth = depth,
                                 .eval        = alpha,
                                 .evalType    = evalType,
                                 .bestMove    = bestMove });

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

using Clock = std::chrono::high_resolution_clock;

Move find_best_move(
    const Options&          options,
    TranspositionTable&     transTable,
    const std::atomic_bool& exitFlag)
{
    assert(options.depth > 0uz);

    const auto searchStartTime = Clock::now();

    std::vector<Move> moves;

    if (options.movesToSearch.empty())
        moves::generate(options.position, std::back_inserter(moves));
    else
        std::ranges::copy(options.movesToSearch, std::back_inserter(moves));

    if (moves.empty()) {
        throw std::invalid_argument {
            std::format(
                "No legal moves in position {}",
                notation::to_fen(options.position))
        };
    }

    const auto numMovesToSearch = options.maxNodes.or_else([numMoves = moves.size()] {
                                                      return std::optional { numMoves };
                                                  })
                                      .value();

    std::optional<Move> bestMove;

    static constexpr auto beta = EVAL_MAX;

    auto alpha = -EVAL_MAX;

    auto depth = 1uz;

    // iterative deepening
    for (; depth <= options.depth; ++depth) {
        const bool shouldExit = [depth, &exitFlag, &options, &searchStartTime] {
            if (depth < 2uz)
                return false;

            if (exitFlag.load())
                return true;

            if (! options.searchTime.has_value())
                return false;

            const auto elapsedMs = std::chrono::duration_cast<Milliseconds>(Clock::now() - searchStartTime);

            return elapsedMs >= *options.searchTime;
        }();

        if (shouldExit)
            break;

        // we can generate the legal moves only once, but we should reorder them each iteration
        // because the move ordering will change based on the evaluations done during the last iteration
        detail::order_moves_for_search(options.position, moves, transTable);

        for (const auto& move : moves | std::views::take(numMovesToSearch)) {
            const auto newPosition = game::after_move(options.position, move);

            const auto score = -alpha_beta(
                -beta, -alpha, newPosition, depth, 1uz, transTable);

            if (score > alpha) {
                bestMove = move;
                alpha    = score;
            }
        }
    }

    transTable.store(options.position, { .searchedDepth = depth,
                                           .eval        = alpha,
                                           .evalType    = EvalType::Exact,
                                           .bestMove    = bestMove });

    return bestMove.value();
}

} // namespace chess::search
