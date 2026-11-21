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
// Reverse futility pruning
// Null move pruning
// Killer moves
// Mate distance pruning
// MultiPV
// Static exchange evaluation
// Late move reductions
// Futility pruning
// Check extensions
// History heuristic
// Late move pruning

#include "search/MoveOrdering.hpp"
#include "search/TimeManagement.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::max(), std::log()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <iterator>
#include <libbenbot/data-structures/HistoryTable.hpp>
#include <libbenbot/data-structures/KillerMoves.hpp>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libbenbot/eval/SEE.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Bounds.hpp>
#include <libbenbot/search/Constants.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libutil/Threading.hpp>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace ben_bot::search {

using chess::moves::MoveList;
using std::size_t;

namespace {

    using eval::Score;

    // null move pruning is only tried when at least this much depth is left
    inline constexpr auto NMP_MIN_DEPTH { 3uz };

    // futility pruning is only done with at most this much depth left
    inline constexpr auto FP_MAX_DEPTH { 2uz };

    // late move pruning is only done with at most this much depth left
    inline constexpr auto LMP_MAX_DEPTH { 7uz };

    // the number of moves to search before late move pruning starts skipping quiet moves.
    // This uses depth as other engines count it, where a frontier node (depthLeft == 0) is
    // depth 1. It uses integer maths only, so that bench is the same on every platform.
    [[nodiscard]] constexpr auto lmp_move_count(const size_t depthLeft) noexcept -> size_t
    {
        const auto depth = depthLeft + 1uz;

        return 3uz + (depth * depth);
    }

    static_assert(lmp_move_count(0uz) == 4uz);
    static_assert(lmp_move_count(1uz) == 7uz);
    static_assert(lmp_move_count(2uz) == 12uz);
    static_assert(lmp_move_count(3uz) == 19uz);
    static_assert(lmp_move_count(4uz) == 28uz);
    static_assert(lmp_move_count(5uz) == 39uz);
    static_assert(lmp_move_count(6uz) == 52uz);
    static_assert(lmp_move_count(LMP_MAX_DEPTH) == 67uz);

    // moves that give check are only extended up to this ply. Reaching MAX_PLY aborts
    // the entire search, so this stops long sequences of checks from getting there.
    inline constexpr auto MAX_EXTENSION_PLY { MAX_PLY / 4uz };

    // used as a zugzwang guard for null move pruning: returns true if the side to move has at
    // least 2 non-pawn pieces. Allowing a single minor piece misses zugzwang mates (see the
    // MatePV unit test).
    [[nodiscard]] auto has_enough_non_pawn_material(const Position& position) noexcept -> bool
    {
        const auto& pieces = position.is_white_to_move()
                               ? position.pieces_for<chess::pieces::Color::White>()
                               : position.pieces_for<chess::pieces::Color::Black>();

        return (pieces.knights | pieces.bishops | pieces.rooks | pieces.queens).count() >= 2uz;
    }

    struct Stats final {
        size_t nodesSearched { 0uz };
        size_t transTableHits { 0uz };
        size_t staticEvals { 0uz };
        size_t betaCutoffs { 0uz };
        size_t mdpCutoffs { 0uz }; // cutoffs due to mate distance pruning
        size_t qDepth { 0uz };     // max depth reached by any quiescence search
    };

    // late move reductions table, indexed by [depth][move index]
    // the formula is similar to Ethereal's: see https://www.chessprogramming.org/Late_Move_Reductions
    constexpr auto LMR_TABLE_SIZE { 64uz };

    using LMRTable = std::array<std::array<size_t, LMR_TABLE_SIZE>, LMR_TABLE_SIZE>;

    const auto lmrTable = [] {
        LMRTable table { };

        for (auto depth = 1uz; depth < LMR_TABLE_SIZE; ++depth) {
            for (auto moveIdx = 1uz; moveIdx < LMR_TABLE_SIZE; ++moveIdx) {
                table[depth][moveIdx] = static_cast<size_t>(
                    0.78 + (std::log(static_cast<double>(depth)) * std::log(static_cast<double>(moveIdx)) / 2.47));
            }
        }

        return table;
    }();

    [[nodiscard]] auto lmr_table_lookup(const size_t depth, const size_t moveIdx) noexcept -> size_t
    {
        static constexpr auto MAX_IDX = LMR_TABLE_SIZE - 1uz;

        return lmrTable[std::min(depth, MAX_IDX)][std::min(moveIdx, MAX_IDX)];
    }

    struct PvList final {
        void update(const Move move, const PvList& child)
        {
            moves.front() = move;

            std::copy_n(
                child.moves.begin(), child.length,
                std::next(moves.begin()));

            length = child.length + 1uz;

            assert(std::cmp_less_equal(length, moves.size()));
        }

        void reset() noexcept { length = 0uz; }

        void to_movelist(MoveList& list) const
        {
            std::copy_n(
                moves.begin(), length,
                std::back_inserter(list));
        }

    private:
        std::array<Move, MAX_PLY> moves { };

        size_t length { 0uz };
    };

    // encapsulates the arguments to the recursive alpha/beta call
    struct AlphaBetaContext final {
        AlphaBetaContext(
            const Bounds bnd, const Position& pos,
            const size_t depthToSearch, const size_t ply,
            TranspositionTable& trans, Interrupter& inter, Stats& statsToUse,
            PvList& parentPV, KillerMoves& killers, HistoryTable& hist,
            const eval::PieceSquareTables& pst)
            : bounds { bnd }
            , position { pos }
            , depthLeft { depthToSearch }
            , plyFromRoot { ply }
            , transTable { trans }
            , interrupter { inter }
            , stats { statsToUse }
            , killerMoves { killers }
            , history { hist }
            , pieceSquareTables { pst }
            , pv { parentPV }
        {
        }

        AlphaBetaContext(const AlphaBetaContext&)            = default;
        AlphaBetaContext& operator=(const AlphaBetaContext&) = delete;

        template <bool PVNode>
        [[nodiscard]] auto alpha_beta() -> Score // NOLINT(readability-function-cognitive-complexity)
        {
            if (interrupter.should_abort(plyFromRoot))
                return { };

            transTable.prefetch(position);

            // it's important that we do this check before probing the transposition table,
            // because the table only contains static evaluations and doesn't consider game
            // history, so its stored evaluations can't detect threefold repetition draws
            if (position.is_threefold_repetition())
                return { };

            if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot)) {
                ++stats.mdpCutoffs;
                return cutoff.value();
            }

            // check if this position has been searched before to at
            // least this depth and within these bounds for non-PV nodes
            if constexpr (not PVNode) {
                if (const auto value = transTable.probe_eval(position, depthLeft, bounds, plyFromRoot)) {
                    ++stats.transTableHits;
                    return { value->first };
                }
            }

            if (position.is_draw()) {
                transTable.store(
                    position, { .searchedDepth = depthLeft,
                                  .eval        = eval::DRAW,
                                  .evalType    = EvalType::Exact,
                                  .bestMove    = std::nullopt });

                return { };
            }

            const bool inCheck = position.is_check();

            // set to true if quiet moves after the first one can't raise alpha
            bool futilityPruning = false;

            if constexpr (not PVNode) {
                if (not inCheck) {
                    const auto staticEval = eval::evaluate(position, pieceSquareTables);

                    // reverse futility pruning
                    const auto margin = 80 * static_cast<int>(depthLeft);

                    if (std::cmp_greater_equal(staticEval.value, bounds.beta.value + margin))
                        return staticEval;

                    // null move pruning: if we can pass our turn and still fail high in a
                    // reduced-depth search, this node is almost certainly a cutoff
                    if (nullMoveAllowed
                        and depthLeft >= NMP_MIN_DEPTH
                        and staticEval >= bounds.beta
                        and not bounds.beta.is_mate()
                        and has_enough_non_pawn_material(position)) {
                        const auto reduction = 3uz + (depthLeft / 3uz); // includes the 1 ply for the null move itself

                        const auto score = -recurse_null(reduction).alpha_beta<false>();

                        if (interrupter.was_aborted())
                            return { };

                        // we return beta here instead of the score, so that we never return an unproven mate score
                        if (score >= bounds.beta)
                            return bounds.beta;
                    }

                    // futility pruning: near the horizon, if a quiet move would need to gain
                    // more than the margin to raise alpha, don't bother searching it
                    if (depthLeft <= FP_MAX_DEPTH and not bounds.alpha.is_mate()) {
                        const auto fpMargin = 100 + (80 * static_cast<int>(depthLeft));

                        futilityPruning = std::cmp_less_equal(staticEval.value + fpMargin, bounds.alpha.value);
                    }
                }
            }

            auto moves = chess::moves::generate(position);

            if (moves.empty() and inCheck) {
                const auto score = Score::mate(plyFromRoot);

                transTable.store(
                    position, { .searchedDepth = depthLeft,
                                  .eval        = score.to_tt(plyFromRoot),
                                  .evalType    = EvalType::Exact,
                                  .bestMove    = std::nullopt });

                return score;
            }

            detail::order_moves_for_search(
                position, moves, transTable, killerMoves.get(plyFromRoot), history);

            auto evalType { EvalType::Alpha };

            std::optional<Move> bestMove;

            const auto killers = killerMoves.get(plyFromRoot);

            auto nextMoveIdx { 0uz };

            // quiet moves that were searched without causing a cutoff
            MoveList quietsSearched;

            const bool lateMovePruning = depthLeft <= LMP_MAX_DEPTH and not inCheck;

            // always at least 4, so the first move is never pruned
            const auto lmpCount = lmp_move_count(depthLeft);

            for (const auto move : moves) {
                const auto moveIdx  = nextMoveIdx++;
                const auto childPos = after_move(position, move);

                if (futilityPruning and moveIdx > 0uz
                    and position.is_quiet(move) and not childPos.is_check())
                    continue;

                // late move pruning: near the horizon, once we've searched enough moves, the
                // remaining quiet moves are unlikely to raise alpha, so we skip them
                if (lateMovePruning and moveIdx >= lmpCount
                    and not bounds.alpha.is_losing_mate()
                    and position.is_quiet(move)
                    and not std::ranges::contains(killers, move)
                    and not childPos.is_check())
                    continue;

                if constexpr (PVNode) {
                    childPV.reset();
                }

                const auto eval = [this, move, &childPos, inCheck, killers, moveIdx] {
                    // check extensions: search moves that give check one ply deeper, so that
                    // we don't drop into the quiescence search in the middle of a forcing line
                    const auto extension = childPos.is_check() and plyFromRoot < MAX_EXTENSION_PLY
                                             ? 1uz
                                             : 0uz;

                    if (depthLeft + extension == 0uz)
                        return -recurse(childPos).quiescence<PVNode>();

                    // late move reductions: search moves late in the ordering at a reduced depth
                    // with a null window, and only re-search at full depth if they beat alpha
                    if (const size_t reduction = get_reduction<PVNode>(move, childPos, moveIdx, inCheck, killers);
                        reduction > 0uz) {
                        const auto reducedScore = -recurse(childPos, true, reduction, extension).alpha_beta<false>();

                        if (reducedScore <= bounds.alpha)
                            return reducedScore;
                    }

                    // only the first move gets a full window. Waiting for a move to raise alpha
                    // instead means that in a PV node where every move fails low, every move is
                    // searched as a PV node.
                    if (moveIdx == 0uz)
                        return -recurse(childPos, false, 0uz, extension).alpha_beta<PVNode>();

                    // principal variation search: first try searching with a null window

                    const auto nullWinScore = -recurse(childPos, true, 0uz, extension).alpha_beta<false>();

                    if constexpr (PVNode) {
                        if (nullWinScore > bounds.alpha)
                            return -recurse(childPos, false, 0uz, extension).alpha_beta<true>();
                    }

                    return nullWinScore;
                }();

                ++stats.nodesSearched;

                if (interrupter.was_aborted())
                    return { };

                if (eval >= bounds.beta) {
                    // mate distance pruning can lower beta to the exact mate score, so
                    // a mating move found in a PV node causes a cutoff here, and must
                    // still be recorded in the PV
                    if constexpr (PVNode) {
                        pv.update(move, childPV);
                    }

                    transTable.store(
                        position, { .searchedDepth = depthLeft,
                                      .eval        = bounds.beta.to_tt(plyFromRoot),
                                      .evalType    = EvalType::Beta,
                                      .bestMove    = move });

                    ++stats.betaCutoffs;

                    if (position.is_quiet(move)) {
                        killerMoves.store(plyFromRoot, move);
                        update_history(move, quietsSearched);
                    }

                    return bounds.beta;
                }

                if (position.is_quiet(move))
                    quietsSearched.emplace_back(move);

                if (eval > bounds.alpha) {
                    bestMove     = move;
                    evalType     = EvalType::Exact;
                    bounds.alpha = eval;

                    if constexpr (PVNode) {
                        pv.update(move, childPV);
                    }
                }
            }

            transTable.store(
                position, { .searchedDepth = depthLeft,
                              .eval        = bounds.alpha.to_tt(plyFromRoot),
                              .evalType    = evalType,
                              .bestMove    = bestMove });

            return bounds.alpha;
        }

    private:
        // gives the move that caused a cutoff a history bonus, and the
        // quiet moves that were searched before it a penalty
        void update_history(const Move cutoffMove, const MoveList& quietsSearched)
        {
            const auto depth = static_cast<int>(depthLeft) + 1;
            const auto bonus = depth * depth;

            const bool whiteToMove = position.is_white_to_move();

            history.update(whiteToMove, cutoffMove, bonus);

            for (const auto move : quietsSearched)
                history.update(whiteToMove, move, -bonus);
        }

        // returns the number of plies by which to reduce the search of the given move
        template <bool PVNode>
        [[nodiscard]] auto get_reduction(
            const Move move, const Position& childPos, const size_t moveIdx, const bool inCheck,
            const std::span<const Move> killers) const -> size_t
        {
            static constexpr auto MIN_DEPTH { 3uz };

            // number of moves to search at full depth before reducing
            static constexpr auto FULL_DEPTH_MOVES { PVNode ? 3uz : 2uz };

            if (depthLeft < MIN_DEPTH or moveIdx < FULL_DEPTH_MOVES or inCheck
                or not position.is_quiet(move)
                or std::ranges::contains(killers, move)
                or childPos.is_check())
                return 0uz;

            auto reduction = lmr_table_lookup(depthLeft, moveIdx);

            // reduce less in PV nodes
            if constexpr (PVNode) {
                if (reduction > 0uz)
                    --reduction;
            }

            // never reduce straight into the quiescence search
            return std::min(reduction, depthLeft - 2uz);
        }

        // searches only captures, with no depth limit, to try to
        // improve the stability of the static evaluation function
        template <bool PVNode>
        [[nodiscard]] auto quiescence() -> Score
        {
            if (interrupter.should_abort(plyFromRoot) or position.is_draw())
                return { };

            stats.qDepth = std::max(stats.qDepth, plyFromRoot);

            if (const auto cutoff = bounds.mate_distance_pruning(plyFromRoot)) {
                ++stats.mdpCutoffs;
                return cutoff.value();
            }

            if (position.is_checkmate())
                return Score::mate(plyFromRoot);

            auto evaluation = eval::evaluate(position, pieceSquareTables);

            ++stats.staticEvals;

            // see if we can get a cutoff (we may not need to generate moves for this position)
            if (evaluation >= bounds.beta) {
                ++stats.betaCutoffs;
                return bounds.beta;
            }

            bounds.alpha = std::max(bounds.alpha, evaluation);

            auto moves = chess::moves::generate<true>(position); // captures only

            detail::order_moves_for_q_search(position, moves);

            const bool inCheck = position.is_check();

            for (const auto move : moves) {
                assert(position.is_capture(move));

                // skip captures that lose material once all recaptures are played out
                if (not inCheck and eval::see(position, move) < 0)
                    continue;

                if constexpr (PVNode) {
                    childPV.reset();
                }

                evaluation = -(recurse(after_move(position, move)).quiescence<PVNode>());

                ++stats.nodesSearched;

                if (interrupter.was_aborted())
                    return { };

                if (evaluation >= bounds.beta) {
                    // as in alpha_beta(), a mating capture may cause a cutoff
                    // here, and must still be recorded in the PV
                    if constexpr (PVNode) {
                        pv.update(move, childPV);
                    }

                    ++stats.betaCutoffs;
                    return bounds.beta;
                }

                if (evaluation > bounds.alpha) {
                    bounds.alpha = evaluation;

                    if constexpr (PVNode) {
                        pv.update(move, childPV);
                    }
                }
            }

            return bounds.alpha;
        }

        [[nodiscard]] auto recurse(
            const Position& childPos, const bool useNullWindow = false,
            const size_t reduction = 0uz, const size_t extension = 0uz)
            -> AlphaBetaContext
        {
            assert(reduction == 0uz or reduction + 1uz < depthLeft);

            const auto depth = depthLeft + extension;

            return { useNullWindow ? bounds.null_window() : bounds.invert(),
                childPos,
                depth > reduction ? depth - 1uz - reduction : 0uz,
                plyFromRoot + 1uz,
                transTable, interrupter, stats, childPV, killerMoves, history, pieceSquareTables };
        }

        // the reduction includes the 1 ply for the null move itself
        [[nodiscard]] auto recurse_null(const size_t reduction) -> AlphaBetaContext
        {
            AlphaBetaContext child {
                bounds.null_window_beta(),
                after_null_move(position),
                depthLeft > reduction ? depthLeft - reduction : 0uz,
                plyFromRoot + 1uz,
                transTable, interrupter, stats, childPV, killerMoves, history, pieceSquareTables
            };

            // don't allow 2 null moves in a row
            child.nullMoveAllowed = false;

            return child;
        }

        Bounds bounds;

        Position position;

        size_t depthLeft { 0uz }; // decreases each iteration, and when this reaches 0, we call the quiescence search

        size_t plyFromRoot { 0uz }; // increases each iteration

        bool nullMoveAllowed { true }; // false for the child of a null move

        TranspositionTable& transTable; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        Interrupter& interrupter; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        Stats& stats; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        KillerMoves& killerMoves; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        HistoryTable& history; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        PvList& pv; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        const eval::PieceSquareTables& pieceSquareTables; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        PvList childPV;
    };

    struct [[nodiscard]] RootSearchResult final {
        Position                  rootPosition;
        MoveList                  pv;
        Score                     bestScore;
        std::vector<Result::Line> otherLines; // only used in MultiPV mode
        Stats                     stats;
        milliseconds              duration { 0 };

        [[nodiscard]] auto to_cb_result(
            const size_t depth, const size_t hashfull) const -> Result
        {
            return {
                .position               = rootPosition,
                .duration               = duration,
                .depth                  = depth,
                .qDepth                 = stats.qDepth,
                .score                  = bestScore,
                .pv                     = pv,
                .otherLines             = otherLines,
                .nodesSearched          = stats.nodesSearched,
                .transpositionTableHits = stats.transTableHits,
                .betaCutoffs            = stats.betaCutoffs,
                .staticEvals            = stats.staticEvals,
                .mdpCutoffs             = stats.mdpCutoffs,
                .hashfull               = hashfull
            };
        }
    };

    // searches all the root moves, except for the excluded moves, to find the best line
    [[nodiscard]] auto search_root_line(
        const size_t                   depth,
        const Options&                 options,
        const MoveList&                excludedMoves,
        const Position&                position,
        TranspositionTable&            transTable,
        Interrupter&                   interrupter,
        KillerMoves&                   killerMoves,
        HistoryTable&                  history,
        const eval::PieceSquareTables& pst,
        const Callbacks&               callbacks,
        Stats&                         stats)
        -> Result::Line
    {
        Bounds   bounds;
        MoveList pv;

        bool foundPV = false;
        auto moveIdx = 0uz;

        for (const auto move : options.movesToSearch) {
            if (std::ranges::find(excludedMoves, move) != excludedMoves.end())
                continue;

            callbacks.root_move(move, moveIdx++);

            PvList childPV;

            auto create_context = [depth, &transTable, &interrupter, &stats, &childPV, &killerMoves, &history, &pst](const Bounds boundsToUse, const Position& pos) {
                return AlphaBetaContext {
                    boundsToUse, pos, depth, 1uz, transTable, interrupter, stats, childPV, killerMoves, history, pst
                };
            };

            // principal variation search: first try searching with a null window
            const auto score = [bounds, &position, move, foundPV, make_context = std::move(create_context)] {
                const auto newPos = after_move(position, move);

                if (not foundPV) {
                    return -make_context(bounds.invert(), newPos)
                                .alpha_beta<true>();
                }

                const auto nullWinScore = -make_context(bounds.null_window(), newPos)
                                               .alpha_beta<false>();

                if (bounds.contains(nullWinScore)) {
                    return -make_context(bounds.invert(), newPos)
                                .alpha_beta<true>();
                }

                return nullWinScore;
            }();

            ++stats.nodesSearched;

            if (score > bounds.alpha) {
                bounds.alpha = score;

                pv.clear();
                pv.emplace_back(move);

                childPV.to_movelist(pv);

                killerMoves.store(0uz, move);

                foundPV = true;
            }

            if (interrupter.was_aborted())
                break;
        }

        return {
            .score = bounds.alpha,
            .pv    = pv
        };
    }

    [[nodiscard]] auto root_search(
        const size_t                   depth,
        Options&                       options,
        const Position&                position,
        TranspositionTable&            transTable,
        Interrupter&                   interrupter,
        KillerMoves&                   killerMoves,
        HistoryTable&                  history,
        const eval::PieceSquareTables& pst,
        const Callbacks&               callbacks)
        -> RootSearchResult
    {
        const Timer timer;

        killerMoves.clear();

        detail::order_moves_for_search(
            position, options.movesToSearch, transTable, killerMoves.get(0uz), history);

        RootSearchResult result;

        result.rootPosition = position;

        // in MultiPV mode, we search the root moves once for each line, each time
        // excluding the first moves of the lines that have already been found
        const auto numLines = std::clamp(options.multiPV, 1uz, options.movesToSearch.size());

        MoveList excludedMoves;

        for (auto lineIdx = 0uz; lineIdx < numLines; ++lineIdx) {
            auto line = search_root_line(
                depth, options, excludedMoves, position, transTable, interrupter, killerMoves, history, pst, callbacks, result.stats);

            if (interrupter.was_aborted() or line.pv.empty())
                break;

            excludedMoves.emplace_back(line.pv.front());

            if (lineIdx == 0uz) {
                result.pv        = std::move(line.pv);
                result.bestScore = line.score;
            } else {
                result.otherLines.emplace_back(std::move(line));
            }
        }

        result.duration = timer.get_duration();

        return result;
    }

    using std::memory_order;

    struct ActiveFlagSetter final {
        explicit ActiveFlagSetter(std::atomic_bool& flag)
            : value { flag }
        {
            value.store(true, memory_order::relaxed);
        }

        ~ActiveFlagSetter() { value.store(false, memory_order::relaxed); }

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
    history.age();
    callbacks.search_start(options);

    // if the movesToSearch was empty, then we search all legal moves
    if (options.movesToSearch.empty()) {
        chess::moves::generate(position, std::back_inserter(options.movesToSearch));

        // we shouldn't have been given a checkmated position to search
        assert(not options.movesToSearch.empty());
    }

    // this can't just check movesToSearch, because searchmoves may have restricted the search to 1
    // move out of many, & then we still want to search it as deep as we were asked to
    const bool onlyOneLegalMove = chess::moves::generate(position).size() == 1uz;

    RootSearchResult result;

    auto totalNodesSearched { 0uz };

    // iterative deepening
    auto depth { 1uz };

    // the last depth that was fully searched, which is the depth we report
    auto completedDepth { 0uz };

    while (options.infinite or pondering.load(memory_order::acquire) or depth <= options.depth) {
        if (interrupter.should_abort(0uz))
            break;

        const auto res = root_search(
            depth, options, position, transTable, interrupter, killerMoves, history, pieceSquareTables, callbacks);

        if (interrupter.was_aborted())
            break;

        result         = res;
        completedDepth = depth;

        totalNodesSearched += result.stats.nodesSearched;

        interrupter.iteration_completed();

        // check if we should abort the search
        if (not(options.infinite or pondering.load(memory_order::acquire))) {
            // check "mate in X" search bound
            if (options.mateIn.has_value() and result.bestScore.is_mate()) {
                if (const auto targetPliesToMate = *options.mateIn * 2uz;
                    result.bestScore.ply_to_mate() <= targetPliesToMate)
                    break;
            }

            // if we've found a forced mate within the search depth, a deeper
            // iteration can't find a shorter one, so don't keep searching
            if (result.bestScore.is_winning_mate()
                and result.bestScore.ply_to_mate() <= depth)
                break;

            // only 1 legal move, don't do a deeper iteration
            if (onlyOneLegalMove)
                break;

            // if we've hit our node limit, don't do a deeper iteration
            if (totalNodesSearched >= options.maxNodes)
                break;

            // once we've used up the soft time limit, don't start a deeper iteration
            if (options.softSearchTime.has_value()
                and interrupter.get_search_duration() >= *options.softSearchTime)
                break;

            // if the iteration we just completed took as much or more time than we
            // have remaining for the search, then don't start a deeper iteration
            // because it would probably get interrupted
            if (const auto remaining = interrupter.get_remaining_time()) {
                if (result.duration >= *remaining)
                    break;
            }
        }

        // without this check, the final info output would be printed twice
        // in the infinite case, we do allow this repetition because we want to print the
        // final output before we're going to spin, then the stop command will print the
        // final info again and the bestmove
        if (depth < options.depth or options.infinite or pondering.load(memory_order::acquire)) {
            callbacks.iteration_complete(
                result.to_cb_result(depth, transTable.hashfull()));
        }

        ++depth;
    } // iterative deepening loop end

    // when in ponder mode, we don't want to exit the search
    // until we've received either a stop or ponderhit command
    if (pondering.load(memory_order::acquire)) {
        util::progressive_backoff([this]() noexcept {
            return exitFlag.load(memory_order::acquire)
                or not pondering.load(memory_order::acquire);
        });
    } else if (options.infinite) {
        util::progressive_backoff([this]() noexcept {
            return exitFlag.load(memory_order::acquire);
        });
    }

    callbacks.search_complete(
        result.to_cb_result(completedDepth, transTable.hashfull()));
}

void Context::wait() const
{
    util::progressive_backoff([this]() noexcept {
        return not activeFlag.load(memory_order::relaxed);
    });
}

} // namespace ben_bot::search
