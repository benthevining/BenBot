/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

#include "MoveOrdering.hpp" // NOLINT(build/include_subdir)
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>   // IWYU pragma: keep - for std::abs()
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

    [[nodiscard, gnu::const]] bool is_winning_mate_score(const int score) noexcept
    {
        return score >= eval::MATE;
    }

    [[nodiscard, gnu::const]] bool is_losing_mate_score(const int score) noexcept
    {
        return score <= -eval::MATE;
    }

    [[nodiscard, gnu::const]] bool is_mate_score(const int score) noexcept
    {
        static constexpr auto MAX_MATE_DEPTH { 1000 };

        return std::abs(score) > EVAL_MAX - MAX_MATE_DEPTH;
    }

    [[nodiscard, gnu::const]] size_t ply_to_mate_from_score(const int score) noexcept
    {
        assert(is_mate_score(score));

        return static_cast<size_t>(EVAL_MAX - std::abs(score));
    }

    // times the search and also watches the "exit" flag
    struct Interrupter final {
        Interrupter(
            const std::atomic_bool&           exitFlagToUse,
            const std::optional<Milliseconds> maxSearchTime)
            : exitFlag { exitFlagToUse }
            , searchTime { maxSearchTime }
        {
        }

        [[nodiscard]] Milliseconds get_search_duration() const
        {
            return std::chrono::duration_cast<Milliseconds>(Clock::now() - startTime);
        }

        [[nodiscard]] bool should_exit() const
        {
            if (exitFlag.load())
                return true;

            if (! searchTime.has_value())
                return false;

            return get_search_duration() >= *searchTime;
        }

    private:
        // On some systems, high_resolution_clock can be unsteady,
        // in which case it's better to fall back to steady_clock
        using Clock = std::conditional_t<
            std::chrono::high_resolution_clock::is_steady,
            std::chrono::high_resolution_clock,
            std::chrono::steady_clock>;

        const std::atomic_bool& exitFlag;

        std::chrono::time_point<Clock> startTime { Clock::now() };

        std::optional<Milliseconds> searchTime;
    };

    // searches only captures, with no depth limit, to try to
    // improve the stability of the static evaluation function
    [[nodiscard]] int quiescence(
        int alpha, int beta,
        const Position&    currentPosition,
        const size_t       plyFromRoot, // increases each iteration (recursion)
        const Interrupter& interrupter)
    {
        assert(beta > alpha);

        if (currentPosition.is_draw())
            return eval::DRAW;

        if (currentPosition.is_checkmate())
            return checkmate_score(plyFromRoot);

        if (interrupter.should_exit())
            return eval::DRAW;

        auto evaluation = eval::evaluate(currentPosition);

        // see if we can get a cutoff (we may not need to generate moves for this position)
        if (evaluation >= beta)
            return beta;

        alpha = std::max(alpha, evaluation);

        // mate distance pruning
        if (is_winning_mate_score(alpha)) {
            const auto mateScore = checkmate_score(plyFromRoot);

            if (mateScore < beta) {
                beta = mateScore;

                if (alpha >= mateScore)
                    return mateScore;
            }
        } else if (is_losing_mate_score(alpha)) {
            const auto mateScore = checkmate_score(plyFromRoot);

            if (mateScore > alpha) {
                alpha = mateScore;

                if (beta <= mateScore)
                    return mateScore;
            }
        }

        auto moves = moves::generate<true>(currentPosition); // captures only

        detail::order_moves_for_q_search(currentPosition, moves);

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            evaluation = -quiescence(
                -beta, -alpha,
                game::after_move(currentPosition, move),
                plyFromRoot + 1uz, interrupter);

            if (evaluation >= beta)
                return beta;

            alpha = std::max(alpha, evaluation);
        }

        return alpha;
    }

    // standard alpha/beta search algorithm
    // this is called in the body of the higher-level iterative deepening loop
    [[nodiscard]] int alpha_beta(
        int alpha, int beta,
        const Position&     currentPosition,
        const size_t        depth,       // this is the depth left to be searched - decreases each iteration, and when this reaches 1, we call the quiescence search
        const size_t        plyFromRoot, // increases each iteration
        TranspositionTable& transTable,
        const Interrupter&  interrupter)
    {
        assert(beta > alpha);

        // it's important that we do this check before probing the transposition table,
        // because the table only contains static evaluations and doesn't consider game
        // history, so its stored evaluations can't detect threefold repetition draws
        if (currentPosition.is_threefold_repetition())
            return eval::DRAW;

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

        // mate distance pruning
        if (is_winning_mate_score(alpha)) {
            const auto mateScore = checkmate_score(plyFromRoot);

            if (mateScore < beta) {
                beta = mateScore;

                if (alpha >= mateScore)
                    return mateScore;
            }
        } else if (is_losing_mate_score(alpha)) {
            const auto mateScore = checkmate_score(plyFromRoot);

            if (mateScore > alpha) {
                alpha = mateScore;

                if (beta <= mateScore)
                    return mateScore;
            }
        }

        detail::order_moves_for_search(currentPosition, moves, transTable);

        auto evalType { EvalType::Alpha };

        std::optional<Move> bestMove;

        for (const auto& move : moves) {
            const auto newPosition = after_move(currentPosition, move);

            const auto eval = depth > 1uz
                                ? -alpha_beta(-beta, -alpha, newPosition, depth - 1uz, plyFromRoot + 1uz, transTable, interrupter)
                                : -quiescence(-beta, -alpha, newPosition, plyFromRoot + 1uz, interrupter);

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

            if (interrupter.should_exit())
                return alpha;
        }

        transTable.store(
            currentPosition, { .searchedDepth = depth,
                                 .eval        = alpha,
                                 .evalType    = evalType,
                                 .bestMove    = bestMove });

        return alpha;
    }

} // namespace

template <bool PrintUCIInfo>
Move Context<PrintUCIInfo>::search()
{
    exitFlag.store(false);

    assert(options.depth > 0uz);

    const Interrupter interrupter { exitFlag, options.searchTime };

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
    const auto numMovesToSearch = options.maxNodes.value_or(options.movesToSearch.size());

    const bool infinite = ! options.searchTime.has_value();

    std::optional<Move> bestMove;

    int bestScore { 0 };

    // iterative deepening
    auto depth = 1uz;

    while (depth <= options.depth) {
        static constexpr auto beta = EVAL_MAX;

        auto alpha = -EVAL_MAX;

        // we can generate the legal moves only once, but we should reorder them each iteration
        // because the move ordering will change based on the evaluations done during the last iteration
        detail::order_moves_for_search(options.position, options.movesToSearch, transTable);

        std::optional<Move> bestMoveThisDepth;

        for (const auto& move : options.movesToSearch | std::views::take(numMovesToSearch)) {
            const auto score = -alpha_beta(
                -beta, -alpha,
                game::after_move(options.position, move),
                depth, 1uz, transTable, interrupter);

            if (score > alpha) {
                bestMoveThisDepth = move;
                alpha             = score;
            }

            if (bestMove.has_value() && interrupter.should_exit()) {
                --depth; // store the last completed depth in the transposition table
                goto end_search;
            }
        }

        assert(bestMoveThisDepth.has_value());

        bestMove  = bestMoveThisDepth;
        bestScore = alpha;

        if (interrupter.should_exit())
            break;

        if (! infinite && is_mate_score(bestScore) && ply_to_mate_from_score(bestScore) <= depth)
            break;

        ++depth;
    }

end_search:
    assert(bestMove.has_value());

    // store the root position evaluation / best move for move ordering of the next search() invocation
    // the evaluation is the evaluation of the position resulting from playing the best move
    transTable.store(options.position, { .searchedDepth = depth,
                                           .eval        = bestScore,
                                           .evalType    = EvalType::Exact,
                                           .bestMove    = bestMove });

    if constexpr (PrintUCIInfo) {
        const auto searchDuration = interrupter.get_search_duration();

        // TODO: nodes searched
        if (is_mate_score(bestScore)) {
            const auto plyToMate = ply_to_mate_from_score(bestScore);
            const auto mateIn    = plyToMate / 2uz; // plies -> moves

            auto mateVal = static_cast<int>(mateIn);

            if (bestScore < 0)
                mateVal *= -1;

            std::println(
                "info depth {} score mate {} time {}",
                depth, mateVal, searchDuration.count());
        } else {
            std::println(
                "info depth {} score cp {} time {}",
                depth, bestScore, searchDuration.count());
        }

        std::println("bestmove {}", notation::to_uci(bestMove.value()));
    }

    return bestMove.value();
}

template struct Context<true>;
template struct Context<false>;

namespace {

    [[nodiscard]] Milliseconds determine_search_time(
        const Milliseconds                timeRemaining,
        const std::optional<Milliseconds> increment,
        const std::optional<size_t>       movesToNextTimeControl)
    {
        const auto inc = increment.value_or(Milliseconds { 0 });

        const auto movesToGo = movesToNextTimeControl.value_or(40uz);

        assert(movesToGo > 0uz);

        return Milliseconds {
            (timeRemaining.count() / movesToGo)
            + (inc.count() / (movesToGo / 10uz))
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
                isWhite ? goOptions.whiteInc : goOptions.blackInc,
                goOptions.movesToGo);
        }
    }
}

} // namespace chess::search
