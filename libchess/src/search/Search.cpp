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
#include <type_traits>
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

    // this function implements both normal alpha/beta search as well as the quiescence search
    // the quiescence search searches only captures, with the goal of evaluating only quiet
    // positions to try and improve the stability of the static evaluation function
    template <bool QuiescenceSearch>
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
        if (const auto eval = transTable.probe_eval(currentPosition, depth, alpha, beta))
            return eval.value();

        if (currentPosition.is_draw()) {
            // conclusive draws: stalemate, threefold repetition, 50-move rule
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval::DRAW,
                                     .evalType    = EvalType::Exact });

            return eval::DRAW;
        }

        auto moves = moves::generate<QuiescenceSearch>(currentPosition);

        if (moves.empty() && currentPosition.is_check()) {
            // checkmate
            const auto eval = checkmate_score(plyFromRoot);

            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval, // TODO: needs scaling/mapping?
                                     .evalType    = EvalType::Exact });

            return eval;
        }

        if constexpr (QuiescenceSearch) {
            // during quiescence search, we first call the static
            // evaluation function to see if we can get a cutoff
            const auto eval = eval::evaluate(currentPosition);

            if (eval >= beta) {
                transTable.store(
                    currentPosition, { .searchedDepth = depth,
                                         .eval        = beta,
                                         .evalType    = EvalType::Beta });

                return beta;
            }

            alpha = std::max(alpha, eval);
        }

        detail::order_moves_for_search(currentPosition, moves, transTable);

        auto evalType { EvalType::Alpha };

        std::optional<Move> bestMove;

        for (const auto& move : moves) {
            const auto newPosition = game::after_move(currentPosition, move);

            // remain quiescence search if we were called as a quiescence search
            // otherwise, switch to quiescence search mode if depth has decremented to 1
            const auto eval = (! QuiescenceSearch && depth > 1uz)
                                ? -alpha_beta<false>(-beta, -alpha, newPosition, depth - 1uz, plyFromRoot + 1uz, transTable)
                                : -alpha_beta<true>(-beta, -alpha, newPosition, depth, plyFromRoot + 1uz, transTable); // NB. don't decrement depth here to avoid uint wraparound issues!

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

// On some systems, high_resolution_clock can be unsteady,
// in which case it's better to fall back to steady_clock
using HighResolutionSteadyClock = std::conditional_t<
    std::chrono::high_resolution_clock::is_steady,
    std::chrono::high_resolution_clock,
    std::chrono::steady_clock>;

Move Context::search()
{
    exitFlag.store(false);

    assert(options.depth > 0uz);

    const auto searchStartTime = HighResolutionSteadyClock::now();

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        [[likely]];

        moves::generate(options.position, std::back_inserter(options.movesToSearch));

        if (options.movesToSearch.empty()) {
            throw std::invalid_argument {
                std::format(
                    "No legal moves in position {}",
                    notation::to_fen(options.position))
            };
        }
    }

    const auto numMovesToSearch = options.maxNodes.or_else([numMoves = options.movesToSearch.size()] {
                                                      return std::optional { numMoves };
                                                  })
                                      .value();

    std::optional<Move> bestMove;

    static constexpr auto beta = EVAL_MAX;

    auto alpha = -EVAL_MAX;

    // iterative deepening
    for (auto depth = 1uz; depth <= options.depth; ++depth) {
        const bool shouldExit = [depth, this, &searchStartTime] {
            if (depth == 1uz)
                return false;

            if (exitFlag.load())
                return true;

            if (! options.searchTime.has_value())
                return false;

            const auto elapsedMs = std::chrono::duration_cast<Milliseconds>(HighResolutionSteadyClock::now() - searchStartTime);

            return elapsedMs >= *options.searchTime;
        }();

        if (shouldExit)
            break;

        // we can generate the legal moves only once, but we should reorder them each iteration
        // because the move ordering will change based on the evaluations done during the last iteration
        detail::order_moves_for_search(options.position, options.movesToSearch, transTable);

        for (const auto& move : options.movesToSearch | std::views::take(numMovesToSearch)) {
            const auto score = -alpha_beta<false>(
                -beta, -alpha,
                game::after_move(options.position, move),
                depth, 1uz, transTable);

            if (score > alpha) {
                bestMove = move;
                alpha    = score;
            }
        }
    }

    // store the root position evaluation / best move for move ordering of the next search() invocation
    // the evaluation is the evaluation of the position resulting from playing the best move

    const auto* bestStored = transTable.find(
        game::after_move(options.position, bestMove.value()));

    assert(bestStored != nullptr);

    transTable.store(options.position, { .searchedDepth = bestStored->searchedDepth,
                                           .eval        = bestStored->eval,
                                           .evalType    = bestStored->evalType,
                                           .bestMove    = bestMove });

    return bestMove.value();
}

} // namespace chess::search
