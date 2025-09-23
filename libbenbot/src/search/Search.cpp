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
#include <vector>

namespace ben_bot::search {

using std::size_t;

namespace {

    using eval::Score;

    struct Stats final {
        size_t nodesSearched { 0uz };
        size_t transTableHits { 0uz };

        size_t betaCutoffs { 0uz };
        size_t mdpCutoffs { 0uz }; // cutoffs due to mate distance pruning
    };

    struct AlphaBetaContext final {
        Bounds bounds;

        Position currentPosition;

        size_t depth { 0uz }; // this is the depth left to be searched - decreases each iteration, and when this reaches 0, we call the quiescence search

        size_t plyFromRoot { 0uz }; // increases each iteration

        TranspositionTable& transTable;

        Interrupter& interrupter;

        Stats& stats;

        [[nodiscard]] AlphaBetaContext recurse(const Position& newPosition) const
        {
            return {
                .bounds          = bounds.invert(),
                .currentPosition = newPosition,
                .depth           = depth - 1uz,
                .plyFromRoot     = plyFromRoot + 1uz,
                .transTable      = transTable,
                .interrupter     = interrupter,
                .stats           = stats
            };
        }
    };

    // searches only captures, with no depth limit, to try to
    // improve the stability of the static evaluation function
    [[nodiscard]] Score quiescence(AlphaBetaContext context)
    {
        if (context.interrupter.should_abort() or context.currentPosition.is_draw())
            return {};

        if (const auto cutoff = context.bounds.mate_distance_pruning(context.plyFromRoot)) {
            ++context.stats.mdpCutoffs;
            return cutoff.value();
        }

        if (context.currentPosition.is_checkmate())
            return Score::mate(context.plyFromRoot);

        auto evaluation = eval::evaluate(context.currentPosition);

        // see if we can get a cutoff (we may not need to generate moves for this position)
        if (evaluation >= context.bounds.beta) {
            ++context.stats.betaCutoffs;
            return context.bounds.beta;
        }

        context.bounds.alpha = std::max(context.bounds.alpha, evaluation);

        auto moves = chess::moves::generate<true>(context.currentPosition); // captures only

        detail::order_moves_for_q_search(context.currentPosition, moves);

        for (const auto& move : moves) {
            assert(context.currentPosition.is_capture(move));

            evaluation = -quiescence(context.recurse(after_move(context.currentPosition, move)));

            if (context.interrupter.was_aborted())
                return {};

            ++context.stats.nodesSearched;

            if (evaluation >= context.bounds.beta) {
                ++context.stats.betaCutoffs;
                return context.bounds.beta;
            }

            context.bounds.alpha = std::max(context.bounds.alpha, evaluation);
        }

        return context.bounds.alpha;
    }

    // standard alpha/beta search algorithm
    // this is called in the body of the higher-level iterative deepening loop
    [[nodiscard]] Score alpha_beta(AlphaBetaContext context)
    {
        if (context.interrupter.should_abort())
            return {};

        context.transTable.prefetch(context.currentPosition);

        // it's important that we do this check before probing the transposition table,
        // because the table only contains static evaluations and doesn't consider game
        // history, so its stored evaluations can't detect threefold repetition draws
        if (context.currentPosition.is_threefold_repetition())
            return {};

        if (const auto cutoff = context.bounds.mate_distance_pruning(context.plyFromRoot)) {
            ++context.stats.mdpCutoffs;
            return cutoff.value();
        }

        // check if this position has been searched before to at
        // least this depth and within these bounds for non-PV nodes
        if (const auto value = context.transTable.probe_eval(context.currentPosition, context.depth, context.bounds)) {
            ++context.stats.transTableHits;
            return Score::from_tt(*value, context.plyFromRoot);
        }

        if (context.currentPosition.is_draw()) {
            context.transTable.store(
                context.currentPosition, { .searchedDepth = context.depth,
                                             .eval        = eval::DRAW,
                                             .evalType    = EvalType::Exact,
                                             .bestMove    = {} });

            return {};
        }

        auto moves = chess::moves::generate(context.currentPosition);

        if (moves.empty() && context.currentPosition.is_check()) {
            context.transTable.store(
                context.currentPosition, { .searchedDepth = context.depth,
                                             .eval        = -eval::MATE,
                                             .evalType    = EvalType::Exact,
                                             .bestMove    = {} });

            return Score::mate(context.plyFromRoot);
        }

        detail::order_moves_for_search(context.currentPosition, moves, context.transTable);

        auto evalType { EvalType::Alpha };

        std::optional<Move> bestMove;

        for (const auto& move : moves) {
            const auto newPosition = after_move(context.currentPosition, move);

            const auto eval = context.depth > 0uz
                                ? -alpha_beta(context.recurse(newPosition))
                                : -quiescence(context.recurse(newPosition));

            if (context.interrupter.should_abort())
                return {};

            ++context.stats.nodesSearched;

            if (eval >= context.bounds.beta) {
                context.transTable.store(
                    context.currentPosition, { .searchedDepth = context.depth,
                                                 .eval        = context.bounds.beta.to_tt(),
                                                 .evalType    = EvalType::Beta,
                                                 .bestMove    = bestMove });

                ++context.stats.betaCutoffs;

                return context.bounds.beta;
            }

            if (eval > context.bounds.alpha) {
                bestMove             = move;
                evalType             = EvalType::Exact;
                context.bounds.alpha = eval;
            }
        }

        context.transTable.store(
            context.currentPosition, { .searchedDepth = context.depth,
                                         .eval        = context.bounds.alpha.to_tt(),
                                         .evalType    = evalType,
                                         .bestMove    = bestMove });

        return context.bounds.alpha;
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

void Context::search() // NOLINT(readability-function-cognitive-complexity)
{
    assert(options.depth > 0uz);
    assert(not activeFlag.load());

    // sets activeFlag to true while inside this function, resets it to false once function exits
    const ActiveFlagSetter activeFlagRAII { activeFlag };

    Interrupter interrupter { exitFlag, pondering, options.searchTime };

    transTable.new_search();

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        chess::moves::generate(options.position, std::back_inserter(options.movesToSearch));

        assert(! options.movesToSearch.empty());
    }

    const bool infinite = not options.is_bounded();

    Stats stats;

    Score bestScore;

    Move bestMove;

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

        for (const auto& move : options.movesToSearch) {
            const auto score = -alpha_beta({ .bounds = bounds.invert(),
                .currentPosition                     = after_move(options.position, move),
                .depth                               = depth,
                .plyFromRoot                         = 1uz,
                .transTable                          = transTable,
                .interrupter                         = interrupter,
                .stats                               = stats });

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

        bestScore = bounds.alpha;

        bestMove = bestMoveThisDepth.value();

        interrupter.iteration_completed();

        if (not(infinite or pondering.load())) {
            // only 1 legal move, don't do a deeper iteration
            if (options.movesToSearch.size() == 1uz) {
                [[unlikely]];
                break;
            }

            // if we've hit our node limit, don't do a deeper iteration
            if (options.maxNodes.has_value()
                && stats.nodesSearched >= *options.maxNodes) {
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

        // prevent the iteration callback from being called right before search_complete()
        // will be called, otherwise the final info string would be printed twice
        if (depth < options.depth) {
            callbacks.iteration_complete({ .pv = transTable.get_best_line(options.position, bestMoveThisDepth.value()),
                .duration                      = interrupter.get_search_duration(),
                .depth                         = depth,
                .score                         = bestScore,
                .nodesSearched                 = stats.nodesSearched,
                .transpositionTableHits        = stats.transTableHits,
                .betaCutoffs                   = stats.betaCutoffs,
                .mdpCutoffs                    = stats.mdpCutoffs,
                .hashfull                      = transTable.hashfull() });
        }

        ++depth;
    } // iterative deepening loop end

    // we want to report the last completed depth
    --depth;

    // when in ponder mode, we don't want to exit the search
    // until we've received either a stop or ponderhit command
    if (pondering.load()) {
        chess::util::progressive_backoff([this] {
            return exitFlag.load() or not pondering.load();
        });
    }

    assert(! bestMove.is_null());

    callbacks.search_complete({ .pv = transTable.get_best_line(options.position, bestMove),
        .duration                   = interrupter.get_search_duration(),
        .depth                      = depth,
        .score                      = bestScore,
        .nodesSearched              = stats.nodesSearched,
        .transpositionTableHits     = stats.transTableHits,
        .betaCutoffs                = stats.betaCutoffs,
        .mdpCutoffs                 = stats.mdpCutoffs,
        .hashfull                   = transTable.hashfull() });
}

void Context::wait() const
{
    chess::util::progressive_backoff([this] {
        return not activeFlag.load();
    });
}

} // namespace ben_bot::search
