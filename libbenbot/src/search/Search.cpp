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

#include "MoveOrdering.hpp"   // NOLINT(build/include_subdir)
#include "TimeManagement.hpp" // NOLINT(build/include_subdir)
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::abs()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <iterator>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/search/Search.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace chess::search {

using std::size_t;

namespace {
    // arbitrary value used as the starting beta value
    // this should be larger than mate, but smaller than
    // the data type's max (to avoid issues with sign flipping)
    constexpr auto EVAL_MAX = eval::MATE * 2;
} // namespace

namespace detail {

    [[nodiscard, gnu::const]] bool is_mate_score(const int score) noexcept
    {
        return std::abs(score) >= eval::MATE;
    }

    [[nodiscard, gnu::const]] size_t ply_to_mate_from_score(const int score) noexcept
    {
        assert(is_mate_score(score));

        return static_cast<size_t>(EVAL_MAX - std::abs(score));
    }

} // namespace detail

namespace {

    using EvalType = TranspositionTable::Record::EvalType;

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

    // maps ply-from-root mate scores to the MATE constant
    [[nodiscard, gnu::const]] int to_tt_score(const int score) noexcept
    {
        if (is_losing_mate_score(score))
            return -eval::MATE;

        if (is_winning_mate_score(score))
            return eval::MATE;

        return score;
    }

    // maps the MATE constant to a ply-from-root mate score
    [[nodiscard, gnu::const]] int from_tt_score(
        const TranspositionTable::ProbedEval& eval,
        const size_t                          plyFromRoot) noexcept
    {
        const auto [score, type] = eval;

        // map MATE constant to a ply-from-root mate score
        if (type == EvalType::Exact) {
            if (score <= -eval::MATE)
                return checkmate_score(plyFromRoot);

            if (score >= eval::MATE)
                return -checkmate_score(plyFromRoot);
        }

        return score;
    }

    struct Bounds final {
        int alpha { -EVAL_MAX };
        int beta { EVAL_MAX };

        Bounds() noexcept = default;

        Bounds(const int alphaToUse, const int betaToUse) noexcept
            : alpha { alphaToUse }
            , beta { betaToUse }
        {
            assert(beta > alpha);
        }

        [[nodiscard]] Bounds invert() const noexcept
        {
            return { -beta, -alpha };
        }

        // if an MDP cutoff is available, returns the cutoff value
        // if this returns nullopt, the search should continue
        [[nodiscard]] std::optional<int> mate_distance_pruning(const size_t plyFromRoot) noexcept
        {
            const auto mateScore = checkmate_score(plyFromRoot);

            if (is_winning_mate_score(alpha)) {
                if (mateScore < beta) {
                    beta = mateScore;

                    if (alpha >= mateScore)
                        return mateScore;
                }

                return std::nullopt;
            }

            if (is_losing_mate_score(alpha)) {
                if (mateScore > alpha) {
                    alpha = mateScore;

                    if (beta <= mateScore)
                        return mateScore;
                }
            }

            return std::nullopt;
        }
    };

    struct Stats final {
        size_t nodesSearched { 0uz };
    };

    // searches only captures, with no depth limit, to try to
    // improve the stability of the static evaluation function
    [[nodiscard]] int quiescence(
        Bounds          bounds,
        const Position& currentPosition,
        const size_t    plyFromRoot, // increases each iteration (recursion)
        Interrupter&    interrupter,
        Stats&          stats)
    {
        if (interrupter.should_abort())
            return eval::DRAW;

        if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot))
            return cutoff.value();

        if (currentPosition.is_draw())
            return eval::DRAW;

        if (currentPosition.is_checkmate())
            return checkmate_score(plyFromRoot);

        auto evaluation = eval::evaluate(currentPosition);

        // see if we can get a cutoff (we may not need to generate moves for this position)
        if (evaluation >= bounds.beta)
            return bounds.beta;

        bounds.alpha = std::max(bounds.alpha, evaluation);

        auto moves = moves::generate<true>(currentPosition); // captures only

        detail::order_moves_for_q_search(currentPosition, moves);

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            evaluation = -quiescence(
                bounds.invert(),
                game::after_move(currentPosition, move),
                plyFromRoot + 1uz, interrupter, stats);

            if (interrupter.was_aborted())
                return eval::DRAW;

            ++stats.nodesSearched;

            if (evaluation >= bounds.beta)
                return bounds.beta;

            bounds.alpha = std::max(bounds.alpha, evaluation);
        }

        return bounds.alpha;
    }

    // standard alpha/beta search algorithm
    // this is called in the body of the higher-level iterative deepening loop
    [[nodiscard]] int alpha_beta(
        Bounds              bounds,
        const Position&     currentPosition,
        const size_t        depth,       // this is the depth left to be searched - decreases each iteration, and when this reaches 0, we call the quiescence search
        const size_t        plyFromRoot, // increases each iteration
        TranspositionTable& transTable,
        Interrupter&        interrupter,
        Stats&              stats)
    {
        if (interrupter.should_abort())
            return eval::DRAW;

        if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot))
            return cutoff.value();

        // it's important that we do this check before probing the transposition table,
        // because the table only contains static evaluations and doesn't consider game
        // history, so its stored evaluations can't detect threefold repetition draws
        if (currentPosition.is_threefold_repetition())
            return eval::DRAW;

        // check if this position has been searched before to at
        // least this depth and within these bounds for non-PV nodes
        if (const auto value = transTable.probe_eval(currentPosition, depth, bounds.alpha, bounds.beta))
            return from_tt_score(*value, plyFromRoot);

        if (currentPosition.is_draw()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval::DRAW,
                                     .evalType    = EvalType::Exact });

            return eval::DRAW;
        }

        auto moves = moves::generate(currentPosition);

        if (moves.empty() && currentPosition.is_check()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = -eval::MATE,
                                     .evalType    = EvalType::Exact });

            return checkmate_score(plyFromRoot);
        }

        detail::order_moves_for_search(currentPosition, moves, transTable);

        auto evalType { EvalType::Alpha };

        std::optional<Move> bestMove;

        for (const auto& move : moves) {
            const auto newPosition = after_move(currentPosition, move);

            const auto eval = depth > 0uz
                                ? -alpha_beta(bounds.invert(), newPosition, depth - 1uz, plyFromRoot + 1uz, transTable, interrupter, stats)
                                : -quiescence(bounds.invert(), newPosition, plyFromRoot + 1uz, interrupter, stats);

            if (interrupter.should_abort())
                return eval::DRAW;

            ++stats.nodesSearched;

            if (eval >= bounds.beta) {
                transTable.store(
                    currentPosition, { .searchedDepth = depth,
                                         .eval        = to_tt_score(bounds.beta),
                                         .evalType    = EvalType::Beta,
                                         .bestMove    = bestMove });

                return bounds.beta;
            }

            if (eval > bounds.alpha) {
                bestMove     = move;
                evalType     = EvalType::Exact;
                bounds.alpha = eval;
            }
        }

        transTable.store(
            currentPosition, { .searchedDepth = depth,
                                 .eval        = to_tt_score(bounds.alpha),
                                 .evalType    = evalType,
                                 .bestMove    = bestMove });

        return bounds.alpha;
    }

    struct ActiveFlagSetter final {
        explicit ActiveFlagSetter(std::atomic_bool& flag)
            : value { flag }
        {
            value.store(true);
        }

        ~ActiveFlagSetter() { value.store(false); }

        ActiveFlagSetter(const ActiveFlagSetter&)            = delete;
        ActiveFlagSetter& operator=(const ActiveFlagSetter&) = delete;
        ActiveFlagSetter(ActiveFlagSetter&&)                 = delete;
        ActiveFlagSetter& operator=(ActiveFlagSetter&&)      = delete;

    private:
        std::atomic_bool& value; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    };

} // namespace

void Context::search()
{
    assert(options.depth > 0uz);
    assert(! activeFlag.load());

    // sets activeFlag to true while inside this function, resets it to false once function exits
    const ActiveFlagSetter activeFlagRAII { activeFlag };

    Interrupter interrupter { exitFlag, options.searchTime };

    if (const auto bookMove = openingBook.get_move(options.position)) {
        const auto eval = eval::evaluate(game::after_move(options.position, *bookMove));

        callbacks.search_complete({ .duration = interrupter.get_search_duration(),
            .depth                            = 1uz,
            .score                            = eval,
            .bestMove                         = *bookMove,
            .nodesSearched                    = 0uz });

        return;
    }

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        moves::generate(options.position, std::back_inserter(options.movesToSearch));

        assert(! options.movesToSearch.empty());
    }

    const bool infinite = ! options.is_bounded();

    Stats stats;

    std::optional<Move> bestMove;

    int bestScore { 0 };

    // iterative deepening
    auto depth = 1uz;

    while (depth <= options.depth) {
        if (interrupter.should_abort())
            break;

        const Timer timer;

        // we can generate the legal moves only once, but we should reorder them each iteration
        // because the move ordering will change based on the evaluations done during the last iteration
        detail::order_moves_for_search(options.position, options.movesToSearch, transTable);

        Bounds bounds {};

        std::optional<Move> bestMoveThisDepth;

        size_t moveNum { 0uz }; // replace with std::enumerate once Clang supports it

        for (const auto& move : options.movesToSearch) {
            callbacks.curr_move({ .move = move,
                .number                 = moveNum });

            ++moveNum;

            const auto score = -alpha_beta(
                bounds.invert(),
                game::after_move(options.position, move),
                depth, 1uz, transTable, interrupter, stats);

            if (interrupter.was_aborted())
                break;

            if (score > bounds.alpha) {
                bestMoveThisDepth = move;
                bounds.alpha      = score;
            }
        }

        if (interrupter.was_aborted()) {
            // we write to the transposition table below, and we want to store
            // the last completed depth as the stored evaluation's depth
            --depth;
            break;
        }

        assert(bestMoveThisDepth.has_value());

        bestMove  = bestMoveThisDepth;
        bestScore = bounds.alpha;

        interrupter.iteration_completed();

        callbacks.iteration_complete({ .duration = interrupter.get_search_duration(),
            .depth                               = depth,
            .score                               = bestScore,
            .bestMove                            = bestMove.value(),
            .nodesSearched                       = stats.nodesSearched });

        if (! infinite) {
            if (options.maxNodes.has_value()
                && stats.nodesSearched >= *options.maxNodes) {
                break;
            }

            // if we've found a mate, don't do a deeper iteration
            if (detail::is_mate_score(bestScore)
                && detail::ply_to_mate_from_score(bestScore) <= depth) {
                break;
            }

            // if the iteration we just completed took as much or more time than we
            // have remaining for the search, then don't start a deeper iteration
            // because it would probably get interrupted
            if (const auto remaining = interrupter.get_remaining_time()) {
                if (timer.get_duration() >= *remaining)
                    break;
            }
        }

        ++depth;
    } // iterative deepening loop end

    assert(bestMove.has_value());

    // store the root position evaluation / best move for move ordering of the next search() invocation
    // the evaluation is the evaluation of the position resulting from playing the best move
    transTable.store(options.position, { .searchedDepth = depth,
                                           .eval        = to_tt_score(bestScore),
                                           .evalType    = EvalType::Exact,
                                           .bestMove    = bestMove });

    callbacks.search_complete({ .duration = interrupter.get_search_duration(),
        .depth                            = depth,
        .score                            = bestScore,
        .bestMove                         = bestMove.value(),
        .nodesSearched                    = stats.nodesSearched });
}

void Context::wait() const
{
    while (activeFlag.load())
        std::this_thread::yield();
}

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
        const bool isWhite = position.is_white_to_move();

        const auto& timeLeft = isWhite ? goOptions.whiteTimeLeft : goOptions.blackTimeLeft;

        // need to know at least our time remaining in order to calculate search time limit
        if (timeLeft.has_value()) {
            searchTime = determine_search_time(
                *timeLeft,
                isWhite ? goOptions.whiteInc : goOptions.blackInc,
                goOptions.movesToGo);
        } else {
            assert(false); // TODO: ??
        }
    }
}

} // namespace chess::search
