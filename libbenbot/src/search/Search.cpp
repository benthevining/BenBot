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

// Search features:
// Alpha-beta pruning with iterative deepening and quiescence search
// Mate distance pruning

#include "MoveOrdering.hpp"
#include "TimeManagement.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::abs()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <iterator>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Bounds.hpp>
#include <libbenbot/search/Context.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/util/Threading.hpp>
#include <optional>

namespace ben_bot::search {

using std::size_t;

namespace {

    using eval::Score;

    struct Stats final {
        size_t nodesSearched { 0uz };
        size_t transTableHits { 0uz };
        size_t staticEvals { 0uz };
        size_t betaCutoffs { 0uz };
        size_t mdpCutoffs { 0uz }; // cutoffs due to mate distance pruning
        size_t qDepth { 0uz };
    };

    // encapsulates the arguments to the recursive alpha/beta call
    struct AlphaBetaContext final {
        AlphaBetaContext(
            const Bounds bnd, const Position& pos,
            const size_t depthLeft, const size_t ply,
            TranspositionTable& trans, Interrupter& inter, Stats& statsToUse)
            : bounds { bnd }
            , position { pos }
            , depth { depthLeft }
            , plyFromRoot { ply }
            , transTable { trans }
            , interrupter { inter }
            , stats { statsToUse }
        {
        }

        // standard alpha/beta search algorithm
        // this is called in the body of the higher-level iterative deepening loop
        [[nodiscard]] auto alpha_beta() -> Score
        {
            if (interrupter.should_abort())
                return {};

            transTable.prefetch(position);

            // it's important that we do this check before probing the transposition table,
            // because the table only contains static evaluations and doesn't consider game
            // history, so its stored evaluations can't detect threefold repetition draws
            if (position.is_threefold_repetition())
                return {};

            if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot)) {
                ++stats.mdpCutoffs;
                return cutoff.value();
            }

            // check if this position has been searched before to at
            // least this depth and within these bounds for non-PV nodes
            if (const auto value = transTable.probe_eval(position, depth, bounds)) {
                ++stats.transTableHits;
                return Score::from_tt(value->first, plyFromRoot);
            }

            if (position.is_draw()) {
                transTable.store(
                    position, { .searchedDepth = depth,
                                  .eval        = eval::DRAW,
                                  .evalType    = EvalType::Exact,
                                  .bestMove    = std::nullopt });

                return {};
            }

            auto moves = chess::moves::generate(position);

            if (moves.empty() && position.is_check()) {
                const auto score = Score::mate(plyFromRoot);

                transTable.store(
                    position, { .searchedDepth = depth,
                                  .eval        = score.to_tt(),
                                  .evalType    = EvalType::Exact,
                                  .bestMove    = std::nullopt });

                return score;
            }

            detail::order_moves_for_search(position, moves, transTable);

            auto evalType { EvalType::Alpha };

            std::optional<Move> bestMove;

            for (const auto& move : moves) {
                auto child = recurse(move);

                const auto eval = depth > 0uz ? -child.alpha_beta() : -child.quiescence();

                if (interrupter.should_abort())
                    return {};

                ++stats.nodesSearched;

                if (eval >= bounds.beta) {
                    transTable.store(
                        position, { .searchedDepth = depth,
                                      .eval        = bounds.beta.to_tt(),
                                      .evalType    = EvalType::Beta,
                                      .bestMove    = bestMove });

                    ++stats.betaCutoffs;

                    return bounds.beta;
                }

                if (eval > bounds.alpha) {
                    bestMove     = move;
                    evalType     = EvalType::Exact;
                    bounds.alpha = eval;
                }
            }

            transTable.store(
                position, { .searchedDepth = depth,
                              .eval        = bounds.alpha.to_tt(),
                              .evalType    = evalType,
                              .bestMove    = bestMove });

            return bounds.alpha;
        }

    private:
        // searches only captures, with no depth limit, to try to
        // improve the stability of the static evaluation function
        [[nodiscard]] auto quiescence() -> Score
        {
            if (interrupter.should_abort() or position.is_draw())
                return {};

            stats.qDepth = std::max(stats.qDepth, plyFromRoot);

            if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot)) {
                ++stats.mdpCutoffs;
                return cutoff.value();
            }

            if (position.is_checkmate())
                return Score::mate(plyFromRoot);

            auto evaluation = eval::evaluate(position);

            ++stats.staticEvals;

            // see if we can get a cutoff (we may not need to generate moves for this position)
            if (evaluation >= bounds.beta) {
                ++stats.betaCutoffs;
                return bounds.beta;
            }

            bounds.alpha = std::max(bounds.alpha, evaluation);

            auto moves = chess::moves::generate<true>(position); // captures only

            detail::order_moves_for_q_search(position, moves);

            for (const auto& move : moves) {
                assert(position.is_capture(move));

                evaluation = -(recurse(move).quiescence());

                if (interrupter.was_aborted())
                    return {};

                ++stats.nodesSearched;

                if (evaluation >= bounds.beta) {
                    ++stats.betaCutoffs;
                    return bounds.beta;
                }

                bounds.alpha = std::max(bounds.alpha, evaluation);
            }

            return bounds.alpha;
        }

        [[nodiscard]] auto recurse(const Move move) const -> AlphaBetaContext
        {
            const auto left = depth > 0uz ? depth - 1uz : 0uz;

            return { bounds.invert(),
                after_move(position, move),
                left, plyFromRoot + 1uz,
                transTable, interrupter, stats };
        }

        Bounds bounds;

        Position position;

        size_t depth { 0uz }; // this is the depth left to be searched - decreases each iteration, and when this reaches 0, we call the quiescence search

        size_t plyFromRoot { 0uz }; // increases each iteration

        TranspositionTable& transTable; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        Interrupter& interrupter; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        Stats& stats; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    };

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

void Context::search() // NOLINT(readability-function-cognitive-complexity)
{
    assert(options.depth > 0uz);
    assert(not activeFlag.load());

    // sets activeFlag to true while inside this function, resets it to false once function exits
    const ActiveFlagSetter activeFlagRAII { activeFlag };

    Interrupter interrupter { exitFlag, pondering, options.searchTime, options.infinite };

    transTable.new_search();

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        chess::moves::generate(options.position, std::back_inserter(options.movesToSearch));

        assert(! options.movesToSearch.empty());
    }

    Stats stats;

    std::optional<Move> bestMove;

    Score bestScore;

    // iterative deepening
    auto depth = 1uz;

    while (depth <= options.depth) {
        if (interrupter.should_abort())
            break;

        const Timer timer;

        // we can generate the legal moves only once, but we should reorder them each iteration
        // because the move ordering will change based on the evaluations done during the last iteration
        detail::order_moves_for_search(options.position, options.movesToSearch, transTable);

        Bounds bounds;

        std::optional<Move> bestMoveThisDepth;

        for (const auto& move : options.movesToSearch) {
            AlphaBetaContext context { bounds.invert(),
                after_move(options.position, move),
                depth, 1uz, transTable, interrupter, stats };

            const auto score = -context.alpha_beta();

            if (interrupter.was_aborted())
                break;

            if (score > bounds.alpha) {
                bestMoveThisDepth = move;
                bounds.alpha      = score;
            }
        }

        if (interrupter.was_aborted())
            break;

        assert(bestMoveThisDepth.has_value());

        bestMove  = bestMoveThisDepth;
        bestScore = bounds.alpha;

        interrupter.iteration_completed();

        if (not(options.infinite or pondering.load())) {
            // check "mate in X" search bound
            if (options.mateIn.has_value() and bestScore.is_mate()) {
                if (const auto targetPliesToMate = *options.mateIn * 2uz;
                    bestScore.ply_to_mate() <= targetPliesToMate)
                    break;
            }

            // only 1 legal move, don't do a deeper iteration
            if (options.movesToSearch.size() == 1uz) {
                [[unlikely]];
                break;
            }

            // if we've hit our node limit, don't do a deeper iteration
            if (stats.nodesSearched >= options.maxNodes)
                break;

            // if the iteration we just completed took as much or more time than we
            // have remaining for the search, then don't start a deeper iteration
            // because it would probably get interrupted
            if (const auto remaining = interrupter.get_remaining_time()) {
                if (timer.get_duration() >= *remaining)
                    break;
            }
        }

        // in the infinite case, we do allow this repetition because we want to print the
        // final output before we're going to spin, then the stop command will print the
        // final info again and the bestmove
        if (depth < options.depth or options.infinite) {
            callbacks.iteration_complete({ .duration = interrupter.get_search_duration(),
                .depth                               = depth,
                .qDepth                              = stats.qDepth,
                .score                               = bestScore,
                .bestMove                            = bestMove.value(),
                .nodesSearched                       = stats.nodesSearched,
                .transpositionTableHits              = stats.transTableHits,
                .betaCutoffs                         = stats.betaCutoffs,
                .staticEvals                         = stats.staticEvals,
                .mdpCutoffs                          = stats.mdpCutoffs,
                .hashfull                            = transTable.hashfull() });
        }

        ++depth;
    } // iterative deepening loop end

    // we want to report the last completed depth
    --depth;

    assert(bestMove.has_value());

    // when in ponder mode, we don't want to exit the search
    // until we've received either a stop or ponderhit command
    if (pondering.load()) {
        chess::util::progressive_backoff([this] {
            return exitFlag.load() or not pondering.load();
        });
    } else if (options.infinite) {
        chess::util::progressive_backoff([this] {
            return exitFlag.load();
        });
    }

    callbacks.search_complete({ .duration = interrupter.get_search_duration(),
        .depth                            = depth,
        .qDepth                           = stats.qDepth,
        .score                            = bestScore,
        .bestMove                         = bestMove.value(),
        .nodesSearched                    = stats.nodesSearched,
        .transpositionTableHits           = stats.transTableHits,
        .betaCutoffs                      = stats.betaCutoffs,
        .staticEvals                      = stats.staticEvals,
        .mdpCutoffs                       = stats.mdpCutoffs,
        .hashfull                         = transTable.hashfull() });
}

void Context::wait() const
{
    chess::util::progressive_backoff([this] {
        return not activeFlag.load();
    });
}

} // namespace ben_bot::search
