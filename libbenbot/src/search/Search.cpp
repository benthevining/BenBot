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
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/search/Search.hpp>
#include <libbenbot/search/TranspositionTable.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/Colors.hpp>
#include <optional>
#include <print>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
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
        // multiply by -1 here because this score is relative to the player who got mated
        return (EVAL_MAX - static_cast<int>(plyFromRoot)) * -1;
    }

    // searches only captures, with no depth limit, to try to
    // improve the stability of the static evaluation function
    [[nodiscard]] int quiescence(
        int alpha, const int beta,
        const Position&     currentPosition,
        const size_t        plyFromRoot, // increases each iteration (recursion)
        TranspositionTable& transTable)
    {
        assert(beta > alpha);

        // In quiescence search, we do not probe the transposition table for a stored evaluation.
        // In regular alpha/beta, we're able to check if an eval was stored with the given depth
        // that we'd be searching the position to, but quiescence search has no depth limit. I
        // tried probing the table for stored results with an arbitrary depth such as 5 or 10,
        // but experimental results showed that not probing the tree here made the bot stronger.

        // This is the depth that we store alongside evaluations in the transposition table for
        // results computed inside this function. In quiescence search mode, we have no depth limit,
        // so I think it makes sense to store a low value here so that non-quiescence searches to
        // depths of 2 and higher will overwrite results stored from quiescence searches (because
        // those searches will consider quiet moves that may be stronger).
        static constexpr auto depth = 1uz;

        if (currentPosition.is_draw()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval::DRAW,
                                     .evalType    = EvalType::Exact });

            return eval::DRAW;
        }

        auto evaluation = eval::evaluate(currentPosition);

        // see if we can get a cutoff (we may not need to generate moves for this position)
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

            evaluation = -quiescence(
                -beta, -alpha,
                game::after_move(currentPosition, move),
                plyFromRoot + 1uz, transTable);

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

    // standard alpha/beta search algorithm
    // this is called in the body of the higher-level iterative deepening loop
    [[nodiscard]] int alpha_beta(
        int alpha, const int beta,
        const Position&     currentPosition,
        const size_t        depth,       // this is the depth left to be searched - decreases each iteration, and when this reaches 1, we call the quiescence search
        const size_t        plyFromRoot, // increases each iteration
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
                                : -quiescence(-beta, -alpha, newPosition, plyFromRoot + 1uz, transTable);

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

template <bool PrintUCIInfo>
Move Context<PrintUCIInfo>::search()
{
    exitFlag.store(false);

    assert(options.depth > 0uz);

    const auto searchStartTime = HighResolutionSteadyClock::now();

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        moves::generate(options.position, std::back_inserter(options.movesToSearch));

        if (options.movesToSearch.empty()) {
            throw std::invalid_argument {
                std::format(
                    "No legal moves in position {}",
                    notation::to_fen(options.position))
            };
        }
    }

    // TODO: I think this is technically supposed to be the max *nodes* to search?
    const auto numMovesToSearch = options.maxNodes.or_else([numMoves = options.movesToSearch.size()] {
                                                      return std::optional { numMoves };
                                                  })
                                      .value();

    std::optional<Move> bestMove;

    static constexpr auto beta = EVAL_MAX;

    auto alpha = -EVAL_MAX;

    // iterative deepening
    auto depth = 1uz;

    for (; depth <= options.depth; ++depth) {
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
            const auto score = -alpha_beta(
                -beta, -alpha,
                game::after_move(options.position, move),
                depth, 1uz, transTable);

            if (score > alpha) {
                bestMove = move;
                alpha    = score;
            }
        }
    }

    const auto score = alpha;

    // store the root position evaluation / best move for move ordering of the next search() invocation
    // the evaluation is the evaluation of the position resulting from playing the best move

    transTable.store(options.position, { .searchedDepth = depth,
                                           .eval        = score,
                                           .evalType    = EvalType::Exact,
                                           .bestMove    = bestMove });

    if constexpr (PrintUCIInfo) {
        const auto endTime = HighResolutionSteadyClock::now();

        const auto searchDuration = std::chrono::duration_cast<Milliseconds>(endTime - searchStartTime);

        // TODO: nodes searched, PV, mate scores
        std::println(
            "info depth {} score cp {} time {}",
            depth, score, searchDuration.count());

        std::println("bestmove {}", notation::to_uci(bestMove.value()));
    }

    return bestMove.value();
}

template struct Context<true>;
template struct Context<false>;

namespace {

    [[nodiscard]] Milliseconds determine_search_time(
        const Milliseconds timeRemaining, const std::optional<Milliseconds> increment) noexcept
    {
        const auto incValue = increment.or_else([] {
                                           return std::optional { Milliseconds { 0 } };
                                       })
                                  .value();

        return Milliseconds {
            (timeRemaining.count() / 20uz)
            + (incValue.count() / 2uz)
        };
    }

} // namespace

void Options::update_from(uci::GoCommandOptions&& goOptions)
{
    // always clear this, because if movesToSearch isn't specified, we
    // want the search algorithm to generate all legal moves instead
    movesToSearch.clear();

    if (! goOptions.moves.empty())
        movesToSearch = std::move(goOptions.moves);

    if (goOptions.depth.has_value())
        depth = *goOptions.depth;

    if (goOptions.nodes.has_value())
        maxNodes = goOptions.nodes;

    // search time
    if (goOptions.searchTime.has_value()) {
        searchTime = goOptions.searchTime;
    } else if (goOptions.infinite) {
        searchTime = std::nullopt;
    } else {
        const bool isWhite = position.sideToMove == pieces::Color::White;

        const auto& timeLeft = isWhite ? goOptions.whiteTimeLeft : goOptions.blackTimeLeft;

        // need to know at least our time remaining in order to calculate search time limit
        if (timeLeft.has_value()) {
            searchTime = determine_search_time(
                *timeLeft,
                isWhite ? goOptions.whiteInc : goOptions.blackInc);
        }
    }
}

} // namespace chess::search
