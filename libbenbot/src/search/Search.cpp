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

    // searches only captures, with no depth limit, to try to
    // improve the stability of the static evaluation function
    [[nodiscard]] Score quiescence(
        Bounds          bounds,
        const Position& currentPosition,
        const size_t    plyFromRoot, // increases each iteration (recursion)
        Interrupter&    interrupter,
        Stats&          stats)
    {
        if (interrupter.should_abort() or currentPosition.is_draw())
            return {};

        if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot)) {
            ++stats.mdpCutoffs;
            return cutoff.value();
        }

        if (currentPosition.is_checkmate())
            return Score::mate(plyFromRoot);

        auto evaluation = eval::evaluate(currentPosition);

        // see if we can get a cutoff (we may not need to generate moves for this position)
        if (evaluation >= bounds.beta) {
            ++stats.betaCutoffs;
            return bounds.beta;
        }

        bounds.alpha = std::max(bounds.alpha, evaluation);

        auto moves = chess::moves::generate<true>(currentPosition); // captures only

        detail::order_moves_for_q_search(currentPosition, moves);

        for (const auto& move : moves) {
            assert(currentPosition.is_capture(move));

            evaluation = -quiescence(
                bounds.invert(),
                after_move(currentPosition, move),
                plyFromRoot + 1uz, interrupter, stats);

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

    // standard alpha/beta search algorithm
    // this is called in the body of the higher-level iterative deepening loop
    [[nodiscard]] Score alpha_beta(
        Bounds              bounds,
        const Position&     currentPosition,
        const size_t        depth,       // this is the depth left to be searched - decreases each iteration, and when this reaches 0, we call the quiescence search
        const size_t        plyFromRoot, // increases each iteration
        TranspositionTable& transTable,
        Interrupter&        interrupter,
        Stats&              stats)
    {
        if (interrupter.should_abort())
            return {};

        transTable.prefetch(currentPosition);

        // it's important that we do this check before probing the transposition table,
        // because the table only contains static evaluations and doesn't consider game
        // history, so its stored evaluations can't detect threefold repetition draws
        if (currentPosition.is_threefold_repetition())
            return {};

        if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot)) {
            ++stats.mdpCutoffs;
            return cutoff.value();
        }

        // check if this position has been searched before to at
        // least this depth and within these bounds for non-PV nodes
        if (const auto value = transTable.probe_eval(currentPosition, depth, bounds)) {
            ++stats.transTableHits;
            return Score::from_tt(*value, plyFromRoot);
        }

        if (currentPosition.is_draw()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = eval::DRAW,
                                     .evalType    = EvalType::Exact,
                                     .bestMove    = {} });

            return {};
        }

        auto moves = chess::moves::generate(currentPosition);

        if (moves.empty() && currentPosition.is_check()) {
            transTable.store(
                currentPosition, { .searchedDepth = depth,
                                     .eval        = -eval::MATE,
                                     .evalType    = EvalType::Exact,
                                     .bestMove    = {} });

            return Score::mate(plyFromRoot);
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
                return {};

            ++stats.nodesSearched;

            if (eval >= bounds.beta) {
                transTable.store(
                    currentPosition, { .searchedDepth = depth,
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
            currentPosition, { .searchedDepth = depth,
                                 .eval        = bounds.alpha.to_tt(),
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

        Bounds bounds {};

        std::optional<Move> bestMoveThisDepth;

        for (const auto& move : options.movesToSearch) {
            const auto score = -alpha_beta(
                bounds.invert(),
                after_move(options.position, move),
                depth, 1uz, transTable, interrupter, stats);

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
            callbacks.iteration_complete({ .pv = transTable.get_best_line(options.position, bestMove.value()),
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

    assert(bestMove.has_value());

    // when in ponder mode, we don't want to exit the search
    // until we've received either a stop or ponderhit command
    if (pondering.load()) {
        chess::util::progressive_backoff([this] {
            return exitFlag.load() or not pondering.load();
        });
    }

    callbacks.search_complete({ .pv = transTable.get_best_line(options.position, bestMove.value()),
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
