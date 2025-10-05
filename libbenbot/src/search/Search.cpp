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
// Alpha-beta pruning
// Iterative deepening
// Quiescence search
// Principal variation search
// Mate distance pruning

#include "MoveOrdering.hpp"
#include "TimeManagement.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::max()
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
        size_t qDepth { 0uz };     // max depth reached by any quiescence search
    };

    struct PvList final {
        void update(const Move move, const PvList& child)
        {
            moves.front() = move;

            std::copy_n(
                child.moves.begin(), child.length,
                std::next(moves.begin()));

            length = child.length + 1;
        }

        void reset() noexcept { length = 0uz; }

        void to_movelist(MoveList& list) const
        {
            std::copy_n(
                moves.begin(), length,
                std::back_inserter(list));
        }

    private:
        std::array<Move, 255uz> moves {};

        size_t length { 0uz };
    };

    // encapsulates the arguments to the recursive alpha/beta call
    struct AlphaBetaContext final {
        AlphaBetaContext(
            const Bounds bnd, const Position& pos,
            const size_t depthToSearch, const size_t ply,
            TranspositionTable& trans, Interrupter& inter, Stats& statsToUse,
            PvList& parentPV)
            : bounds { bnd }
            , position { pos }
            , depthLeft { depthToSearch }
            , plyFromRoot { ply }
            , transTable { trans }
            , interrupter { inter }
            , stats { statsToUse }
            , pv { parentPV }
        {
        }

        [[nodiscard]] auto alpha_beta() -> Score // NOLINT(readability-function-cognitive-complexity)
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
            if (const auto value = transTable.probe_eval(position, depthLeft, bounds)) {
                ++stats.transTableHits;
                return Score::from_tt(value->first, plyFromRoot);
            }

            if (position.is_draw()) {
                transTable.store(
                    position, { .searchedDepth = depthLeft,
                                  .eval        = eval::DRAW,
                                  .evalType    = EvalType::Exact,
                                  .bestMove    = std::nullopt });

                return {};
            }

            auto moves = chess::moves::generate(position);

            if (moves.empty() and position.is_check()) {
                const auto score = Score::mate(plyFromRoot);

                transTable.store(
                    position, { .searchedDepth = depthLeft,
                                  .eval        = score.to_tt(),
                                  .evalType    = EvalType::Exact,
                                  .bestMove    = std::nullopt });

                return score;
            }

            detail::order_moves_for_search(position, moves, transTable);

            auto evalType { EvalType::Alpha };

            std::optional<Move> bestMove;

            for (const auto move : moves) {
                childPV.reset();

                // NB. adding PVS here seems to lose some Elo
                const auto eval = depthLeft > 0uz ? -recurse(move).alpha_beta() : -recurse(move).quiescence();

                ++stats.nodesSearched;

                if (interrupter.was_aborted())
                    return {};

                if (eval >= bounds.beta) {
                    transTable.store(
                        position, { .searchedDepth = depthLeft,
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

                    pv.update(move, childPV);
                }
            }

            transTable.store(
                position, { .searchedDepth = depthLeft,
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

            for (const auto move : moves) {
                assert(position.is_capture(move));

                evaluation = -(recurse(move).quiescence());

                ++stats.nodesSearched;

                if (interrupter.was_aborted())
                    return {};

                if (evaluation >= bounds.beta) {
                    ++stats.betaCutoffs;
                    return bounds.beta;
                }

                bounds.alpha = std::max(bounds.alpha, evaluation);
            }

            return bounds.alpha;
        }

        [[nodiscard]] auto recurse(const Move move) -> AlphaBetaContext
        {
            return { bounds.invert(),
                after_move(position, move),
                depthLeft > 0uz ? depthLeft - 1uz : 0uz,
                plyFromRoot + 1uz,
                transTable, interrupter, stats, childPV };
        }

        Bounds bounds;

        Position position;

        size_t depthLeft { 0uz }; // decreases each iteration, and when this reaches 0, we call the quiescence search

        size_t plyFromRoot { 0uz }; // increases each iteration

        TranspositionTable& transTable; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        Interrupter& interrupter; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        Stats& stats; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        PvList& pv; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        PvList childPV;
    };

    struct RootSearchResult final {
        MoveList     pv;
        Score        bestScore;
        Stats        stats;
        milliseconds duration { 0 };

        [[nodiscard]] auto to_cb_result(
            const size_t depth, const size_t hashfull) const noexcept -> Result
        {
            return { .duration          = duration,
                .depth                  = depth,
                .qDepth                 = stats.qDepth,
                .score                  = bestScore,
                .pv                     = pv,
                .nodesSearched          = stats.nodesSearched,
                .transpositionTableHits = stats.transTableHits,
                .betaCutoffs            = stats.betaCutoffs,
                .staticEvals            = stats.staticEvals,
                .mdpCutoffs             = stats.mdpCutoffs,
                .hashfull               = hashfull };
        }
    };

    [[nodiscard]] auto root_search(
        const size_t        depth,
        Options&            options,
        TranspositionTable& transTable,
        Interrupter&        interrupter)
        -> RootSearchResult
    {
        const Timer timer;

        detail::order_moves_for_search(options.position, options.movesToSearch, transTable);

        Stats    stats;
        Bounds   bounds;
        MoveList pv; // NOLINT(readability-identifier-length)

        bool foundPV = false;

        for (const auto move : options.movesToSearch) {
            PvList childPV;

            // principal variation search: first try searching with a null window
            const auto score = [bounds, &options, move, depth, foundPV, &transTable, &interrupter, &stats, &childPV] {
                const auto newPos = after_move(options.position, move);

                AlphaBetaContext context { bounds.invert(),
                    newPos, depth, 1uz, transTable, interrupter, stats, childPV };

                if (not foundPV)
                    return -context.alpha_beta();

                AlphaBetaContext nullWindowContext { bounds.null_window(),
                    newPos, depth, 1uz, transTable, interrupter, stats, childPV };

                const auto nullWinScore = -nullWindowContext.alpha_beta();

                if (bounds.contains(nullWinScore))
                    return -context.alpha_beta();

                return nullWinScore;
            }();

            ++stats.nodesSearched;

            if (score > bounds.alpha) {
                bounds.alpha = score;

                pv.clear();
                pv.emplace_back(move);

                childPV.to_movelist(pv);

                foundPV = true;
            }

            if (interrupter.was_aborted())
                break;
        }

        return {
            .pv        = pv,
            .bestScore = bounds.alpha,
            .stats     = stats,
            .duration  = timer.get_duration()
        };
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

    Interrupter interrupter { exitFlag, pondering, options.searchTime, options.infinite };

    transTable.new_search();

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        chess::moves::generate(options.position, std::back_inserter(options.movesToSearch));

        assert(! options.movesToSearch.empty());
    }

    RootSearchResult result;

    size_t totalNodesSearched { 0uz };

    // iterative deepening
    auto depth = 1uz;

    while (depth <= options.depth) {
        if (interrupter.should_abort())
            break;

        const auto res = root_search(depth, options, transTable, interrupter);

        if (interrupter.was_aborted())
            break;

        result = res;

        totalNodesSearched += result.stats.nodesSearched;

        interrupter.iteration_completed();

        if (not(options.infinite or pondering.load())) {
            // check "mate in X" search bound
            if (options.mateIn.has_value() and result.bestScore.is_mate()) {
                if (const auto targetPliesToMate = *options.mateIn * 2uz;
                    result.bestScore.ply_to_mate() <= targetPliesToMate)
                    break;
            }

            // only 1 legal move, don't do a deeper iteration
            if (options.movesToSearch.size() == 1uz) {
                [[unlikely]];
                break;
            }

            // if we've hit our node limit, don't do a deeper iteration
            if (totalNodesSearched >= options.maxNodes)
                break;

            // if the iteration we just completed took as much or more time than we
            // have remaining for the search, then don't start a deeper iteration
            // because it would probably get interrupted
            if (const auto remaining = interrupter.get_remaining_time()) {
                if (result.duration >= *remaining)
                    break;
            }
        }

        // in the infinite case, we do allow this repetition because we want to print the
        // final output before we're going to spin, then the stop command will print the
        // final info again and the bestmove
        if (depth < options.depth or options.infinite) {
            callbacks.iteration_complete(
                result.to_cb_result(depth, transTable.hashfull()));
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
    } else if (options.infinite) {
        chess::util::progressive_backoff([this] {
            return exitFlag.load();
        });
    }

    callbacks.search_complete(
        result.to_cb_result(depth, transTable.hashfull()));
}

void Context::wait() const
{
    chess::util::progressive_backoff([this] {
        return not activeFlag.load();
    });
}

} // namespace ben_bot::search
