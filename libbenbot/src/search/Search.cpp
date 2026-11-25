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
// Continuation history (1 ply)
// Late move pruning
// SEE pruning
// Internal iterative reductions
// Aspiration windows

#include "search/MoveOrdering.hpp"
#include "search/TimeManagement.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::max(), std::log()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <iterator>
#include <libbenbot/data-structures/ContinuationHistory.hpp>
#include <libbenbot/data-structures/HistoryTable.hpp>
#include <libbenbot/data-structures/KillerMoves.hpp>
#include <libbenbot/data-structures/PawnTable.hpp>
#include <libbenbot/data-structures/SearchStack.hpp>
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
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
    namespace piece_values = eval::piece_values;

    // null move pruning is only tried when at least this much depth is left
    inline constexpr auto NMP_MIN_DEPTH { 3uz };

    // futility pruning is only done with at most this much depth left
    inline constexpr auto FP_MAX_DEPTH { 2uz };

    // late move pruning is only done with at most this much depth left
    inline constexpr auto LMP_MAX_DEPTH { 7uz };

    // history pruning skips quiet moves that LMR would leave less than this much depth...
    inline constexpr auto HISTORY_PRUNING_MAX_LMR_DEPTH { 3 };

    // ...and whose combined history is below this much per ply of depth, negated
    inline constexpr auto HISTORY_PRUNING_MARGIN { 3072 };

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

    // SEE pruning is only done with at most this much depth left
    inline constexpr auto SEE_PRUNING_MAX_DEPTH { 7uz };

    // moves whose SEE is below this are skipped by SEE pruning. This uses depth as other
    // engines count it, where a frontier node (depthLeft == 0) is depth 1. The thresholds are
    // never positive and never go up with depth, so a move pruned at some depth would also be
    // pruned at every lower depth (for captures, down to the quiescence search's SEE < 0 skip).
    // Captures use a linear threshold: for example, a knight taking a pawn that's defended by
    // a pawn (SEE -220) is pruned at depths 1 and 2. Quiet moves use a quadratic one, so that
    // they're pruned more readily very close to the horizon, where a quiet move has little time
    // to make up for the material it loses, but less readily further away: a quiet move that
    // hangs a pawn (SEE -100) is pruned at depths 1 and 2, a minor piece at depths 1 to 4, and
    // a rook at depths 1 to 5.
    [[nodiscard]] constexpr auto see_pruning_threshold(
        const size_t depthLeft, const bool quiet) noexcept -> int
    {
        const auto depth = static_cast<int>(depthLeft) + 1;

        if (quiet)
            return -15 * depth * depth;

        return -100 * depth;
    }

    static_assert(see_pruning_threshold(0uz, false) == -100);
    static_assert(see_pruning_threshold(1uz, false) == -200);
    static_assert(see_pruning_threshold(2uz, false) == -300);
    static_assert(see_pruning_threshold(3uz, false) == -400);
    static_assert(see_pruning_threshold(4uz, false) == -500);
    static_assert(see_pruning_threshold(5uz, false) == -600);
    static_assert(see_pruning_threshold(6uz, false) == -700);
    static_assert(see_pruning_threshold(SEE_PRUNING_MAX_DEPTH, false) == -800);

    static_assert(see_pruning_threshold(0uz, true) == -15);
    static_assert(see_pruning_threshold(1uz, true) == -60);
    static_assert(see_pruning_threshold(2uz, true) == -135);
    static_assert(see_pruning_threshold(3uz, true) == -240);
    static_assert(see_pruning_threshold(4uz, true) == -375);
    static_assert(see_pruning_threshold(5uz, true) == -540);
    static_assert(see_pruning_threshold(6uz, true) == -735);
    static_assert(see_pruning_threshold(SEE_PRUNING_MAX_DEPTH, true) == -960);

    // a lower bound on the SEE of a capture or promotion, without playing out the exchange:
    // the side making the move can always stop after the opponent's first recapture, so it
    // can't do worse than winning what it takes and then losing the piece it moved there
    [[nodiscard]] auto see_lower_bound(const Position& position, const Move move) noexcept -> int
    {
        const int captured = position.is_en_passant(move)
                               ? piece_values::PAWN
                               : position.their_pieces()
                                     .get_piece_on(move.to())
                                     .transform(piece_values::get)
                                     .value_or(eval::Value { 0 });

        // a promoted piece gains (promoted - pawn), then the promoted piece can be lost
        if (move.is_promotion())
            return captured - piece_values::PAWN;

        return captured - piece_values::get(move.piece());
    }

    // internal iterative reductions are only done with at least this much depth left
    inline constexpr auto IIR_MIN_DEPTH { 4uz };

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

    // used for debug assertions: returns true if the given move could have been the last move
    // played to reach this position, i.e. the piece it moved (or promoted to) is on its target
    // square & belongs to the side that just moved. Null moves are always allowed.
    [[nodiscard, maybe_unused]] auto could_be_last_move(const Position& position, const Move move) noexcept -> bool
    {
        if (move.is_null())
            return true;

        const auto expected = move.promoted_type().value_or(move.piece());

        return position.their_pieces().get_piece_on(move.to()) == expected;
    }

    struct Stats final {
        size_t nodesSearched { 0uz };
        size_t transTableHits { 0uz };
        size_t staticEvals { 0uz };
        size_t betaCutoffs { 0uz };
        size_t mdpCutoffs { 0uz }; // cutoffs due to mate distance pruning
        size_t qDepth { 0uz };     // max depth reached by any quiescence search

        // nodes spent searching the best root move of the first line, & all the root moves of
        // the first line, used by the time management. With an aspiration window, these only
        // count the last search of the first line, not the ones that failed
        size_t bestMoveNodes { 0uz };
        size_t firstLineNodes { 0uz };
    };

    // aspiration windows are only used once this many iterations have been completed
    constexpr auto ASPIRATION_MIN_DEPTH { 4uz };

    // the window's initial half-width is this many centipawns, plus more for bigger scores (see
    // aspiration_delta()). Each time the search fails, the window is widened on the side that
    // failed by delta, & delta grows by half
    constexpr auto ASPIRATION_BASE_DELTA { 10 };

    // past this score (either way), the window is opened all the way on that side, since the
    // score is big enough that it's probably winning or losing anyway, or heading for a mate
    constexpr auto ASPIRATION_MAX_SCORE { 500 };

    // each fail high reduces the depth of the re-search by 1, up to this many plies
    constexpr auto ASPIRATION_MAX_FAIL_HIGH_REDUCTION { 3uz };

    // scores far from 0 swing more between iterations, so they get a wider window: 10cp up to
    // 127, 15 at 300 & 25 at 500. This uses integer maths only, so that bench is the same
    // on every platform
    [[nodiscard]] constexpr auto aspiration_delta(const int score) noexcept -> int
    {
        return ASPIRATION_BASE_DELTA + ((score * score) / 16384);
    }

    static_assert(aspiration_delta(0) == 10);
    static_assert(aspiration_delta(100) == 10);
    static_assert(aspiration_delta(-300) == 15);
    static_assert(aspiration_delta(ASPIRATION_MAX_SCORE) == 25);

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
            ContinuationHistory& contHist,
            PawnTable& pawns, SearchStackEntry* stackEntry)
            : bounds { bnd }
            , position { pos }
            , depthLeft { depthToSearch }
            , plyFromRoot { ply }
            , transTable { trans }
            , interrupter { inter }
            , stats { statsToUse }
            , killerMoves { killers }
            , history { hist }
            , continuationHistory { contHist }
            , pv { parentPV }
            , pawnTable { pawns }
            , ss { stackEntry }
        {
            assert(ss != nullptr);
            assert(could_be_last_move(position, ss->move));
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

            // internal iterative reductions: with no TT move our move ordering is probably poor,
            // so search this node one ply shallower. The next visit should then have a TT move.
            if (not inCheck
                and depthLeft >= IIR_MIN_DEPTH
                and not transTable.find(position).and_then(&TTData::bestMove).has_value())
                --depthLeft;

            // set to true if quiet moves after the first one can't raise alpha
            bool futilityPruning = false;

            // nodes in check don't have a static eval. Every other node works it out, PV nodes
            // included, and stores it before any children are searched, so that the nodes 2 & 4
            // plies below this one can tell whether they're improving on it
            if (not inCheck)
                ss->staticEval = eval::evaluate(position, &pawnTable);

            if constexpr (not PVNode) {
                if (not inCheck) {
                    const auto staticEval = ss->staticEval.value();

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

                        // a mate score from the null move search is unproven, so we return beta instead
                        if (score >= bounds.beta)
                            return score.is_mate() ? bounds.beta : score;
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
                position, moves, transTable, killerMoves.get(plyFromRoot), history,
                continuationHistory, ss->move, std::prev(ss)->move);

            auto evalType { EvalType::Alpha };

            std::optional<Move> bestMove;

            // the best score of any move searched so far, which may be below alpha. The search is
            // fail-soft, so this (rather than alpha or beta) is what the node returns
            std::optional<Score> bestScore;

            const auto killers = killerMoves.get(plyFromRoot);

            auto nextMoveIdx { 0uz };

            // quiet moves that were searched without causing a cutoff
            MoveList quietsSearched;

            const bool lateMovePruning = depthLeft <= LMP_MAX_DEPTH and not inCheck;

            // always at least 4, so the first move is never pruned
            const auto lmpCount = lmp_move_count(depthLeft);

            const bool historyPruning = not inCheck;

            const bool seePruning = depthLeft <= SEE_PRUNING_MAX_DEPTH and not inCheck;

            const auto captureSeeThreshold = see_pruning_threshold(depthLeft, false);
            const auto quietSeeThreshold   = see_pruning_threshold(depthLeft, true);

            for (const auto move : moves) {
                const auto moveIdx  = nextMoveIdx++;
                const auto childPos = after_move(position, move);

                // the search is fail-soft, so if every move searched so far gets mated, that mate
                // score is what this node returns. If we pruned the other moves, that would claim
                // a mate that hasn't been proven, so we don't prune until we've found a better move
                const bool bestIsLosingMate = bestScore.has_value() and bestScore->is_losing_mate();

                if (futilityPruning and moveIdx > 0uz
                    and not bestIsLosingMate
                    and position.is_quiet(move) and not childPos.is_check())
                    continue;

                // late move pruning: near the horizon, once we've searched enough moves, the
                // remaining quiet moves are unlikely to raise alpha, so we skip them
                if (lateMovePruning and moveIdx >= lmpCount
                    and not bounds.alpha.is_losing_mate()
                    and not bestIsLosingMate
                    and position.is_quiet(move)
                    and not std::ranges::contains(killers, move)
                    and not childPos.is_check())
                    continue;

                // history pruning: if LMR would leave a quiet move very little depth, and its
                // history says it's rarely been any good, we skip it. This uses depth as other
                // engines count it, where a frontier node (depthLeft == 0) is depth 1
                if (historyPruning and moveIdx > 0uz
                    and not bounds.alpha.is_losing_mate()
                    and not bestIsLosingMate
                    and position.is_quiet(move)
                    and not std::ranges::contains(killers, move)
                    and not childPos.is_check()) {
                    const auto depth    = static_cast<int>(depthLeft) + 1;
                    const auto lmrDepth = depth - 1 - static_cast<int>(lmr_table_lookup(depthLeft, moveIdx));

                    if (lmrDepth < HISTORY_PRUNING_MAX_LMR_DEPTH
                        and history_score(move) < HISTORY_PRUNING_MARGIN * -depth)
                        continue;
                }

                // SEE pruning: near the horizon, skip moves that lose too much material once all
                // recaptures are played out. For captures, we only call see() if the cheap lower
                // bound can't rule this out, so captures of equal or more valuable pieces never pay
                // for it. Killers aren't exempt (unlike in LMP): a move that loses material by SEE
                // isn't saved by having caused a cutoff in a sibling position
                if (seePruning and moveIdx > 0uz
                    and not bounds.alpha.is_losing_mate()
                    and not bestIsLosingMate
                    and not childPos.is_check()) {
                    if (position.is_quiet(move)) {
                        if (eval::see(position, move) < quietSeeThreshold)
                            continue;
                    } else if (see_lower_bound(position, move) < captureSeeThreshold
                               and eval::see(position, move) < captureSeeThreshold) {
                        continue;
                    }
                }

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
                        return -recurse(childPos, move).quiescence<PVNode>();

                    // late move reductions: search moves late in the ordering at a reduced depth
                    // with a null window, and only re-search at full depth if they beat alpha
                    if (const size_t reduction = get_reduction<PVNode>(move, childPos, moveIdx, inCheck, killers);
                        reduction > 0uz) {
                        const auto reducedScore = -recurse(childPos, move, true, reduction, extension).alpha_beta<false>();

                        if (reducedScore <= bounds.alpha)
                            return reducedScore;
                    }

                    // only the first move gets a full window. Waiting for a move to raise alpha
                    // instead means that in a PV node where every move fails low, every move is
                    // searched as a PV node.
                    if (moveIdx == 0uz)
                        return -recurse(childPos, move, false, 0uz, extension).alpha_beta<PVNode>();

                    // principal variation search: first try searching with a null window

                    const auto nullWinScore = -recurse(childPos, move, true, 0uz, extension).alpha_beta<false>();

                    if constexpr (PVNode) {
                        if (nullWinScore > bounds.alpha)
                            return -recurse(childPos, move, false, 0uz, extension).alpha_beta<true>();
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
                                      .eval        = eval.to_tt(plyFromRoot),
                                      .evalType    = EvalType::Beta,
                                      .bestMove    = move });

                    ++stats.betaCutoffs;

                    if (position.is_quiet(move)) {
                        killerMoves.store(plyFromRoot, move);
                        update_history(move, quietsSearched);
                    }

                    return eval;
                }

                if (position.is_quiet(move))
                    quietsSearched.emplace_back(move);

                if (not bestScore.has_value() or eval > *bestScore)
                    bestScore = eval;

                if (eval > bounds.alpha) {
                    bestMove     = move;
                    evalType     = EvalType::Exact;
                    bounds.alpha = eval;

                    if constexpr (PVNode) {
                        pv.update(move, childPV);
                    }
                }
            }

            // the first move is never pruned, so at least one move was searched
            assert(bestScore.has_value());

            const auto score = bestScore.value_or(bounds.alpha);

            transTable.store(
                position, { .searchedDepth = depthLeft,
                              .eval        = score.to_tt(plyFromRoot),
                              .evalType    = evalType,
                              .bestMove    = bestMove });

            return score;
        }

    private:
        // gives the move that caused a cutoff a history bonus, and the
        // quiet moves that were searched before it a penalty
        void update_history(const Move cutoffMove, const MoveList& quietsSearched)
        {
            // each update moves a score by up to about 10% of the table's maximum. A bonus of
            // depth * depth was so small next to the maximum that the scores barely moved
            static constexpr auto MAX_BONUS { 1600 };

            const auto depth = static_cast<int>(depthLeft) + 1;
            const auto bonus = std::min((16 * depth * depth) + (128 * (depth - 1)), MAX_BONUS);

            const bool whiteToMove = position.is_white_to_move();

            // the opponent's move that led to this node, & our own move before that. Either is
            // null after a null move (or before the root), & then its continuation isn't updated
            const auto prevMove    = ss->move;
            const auto ownPrevMove = std::prev(ss)->move;

            const auto update = [this, whiteToMove, prevMove, ownPrevMove](const Move move, const int amount) {
                history.update(whiteToMove, move, amount);
                continuationHistory.update(not whiteToMove, prevMove, whiteToMove, move, amount);
                continuationHistory.update(whiteToMove, ownPrevMove, whiteToMove, move, amount);
            };

            update(cutoffMove, bonus);

            for (const auto move : quietsSearched)
                update(move, -bonus);
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

            // each this much combined history score changes the reduction by 1 ply, up to
            // MAX_HISTORY_ADJUSTMENT plies either way. Most quiets that are searched get history
            // penalties, so about half of the moves we reduce have a combined score of -8192 or
            // less, and without a limit they'd be reduced far more than the table says
            static constexpr auto HISTORY_DIVISOR { 8192 };
            static constexpr auto MAX_HISTORY_ADJUSTMENT { 2 };

            auto reduction = static_cast<int>(lmr_table_lookup(depthLeft, moveIdx));

            // reduce less in PV nodes
            if constexpr (PVNode) {
                if (reduction > 0)
                    --reduction;
            }

            // reduce more if our static eval isn't better than it was at our previous turn,
            // since the moves here are then less likely to raise alpha
            if (not is_improving(ss))
                ++reduction;

            // reduce moves with a good history less, and moves with a bad history more
            reduction -= std::clamp(
                history_score(move) / HISTORY_DIVISOR,
                -MAX_HISTORY_ADJUSTMENT, MAX_HISTORY_ADJUSTMENT);

            if (reduction <= 0)
                return 0uz;

            // never reduce straight into the quiescence search
            return std::min(static_cast<size_t>(reduction), depthLeft - 2uz);
        }

        // the main & continuation histories of a quiet move, summed the same way as in move ordering
        [[nodiscard]] auto history_score(const Move move) const -> int
        {
            const bool whiteToMove = position.is_white_to_move();

            return history.get(whiteToMove, move)
                 + continuationHistory.get(not whiteToMove, ss->move, whiteToMove, move)
                 + continuationHistory.get(whiteToMove, std::prev(ss)->move, whiteToMove, move);
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

            const bool inCheck = position.is_check();

            auto evaluation = eval::evaluate(position, &pawnTable);

            ++stats.staticEvals;

            if (not inCheck)
                ss->staticEval = evaluation;

            // see if we can get a cutoff (we may not need to generate moves for this position)
            if (evaluation >= bounds.beta) {
                ++stats.betaCutoffs;
                return evaluation;
            }

            // the stand pat score is a lower bound, since we can always choose not to capture
            auto bestScore = evaluation;

            bounds.alpha = std::max(bounds.alpha, evaluation);

            auto moves = chess::moves::generate<true>(position); // captures only

            detail::order_moves_for_q_search(position, moves);

            for (const auto move : moves) {
                assert(position.is_capture(move));

                // skip captures that lose material once all recaptures are played out
                if (not inCheck and eval::see(position, move) < 0)
                    continue;

                if constexpr (PVNode) {
                    childPV.reset();
                }

                evaluation = -(recurse(after_move(position, move), move).quiescence<PVNode>());

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
                    return evaluation;
                }

                bestScore = std::max(bestScore, evaluation);

                if (evaluation > bounds.alpha) {
                    bounds.alpha = evaluation;

                    if constexpr (PVNode) {
                        pv.update(move, childPV);
                    }
                }
            }

            return bestScore;
        }

        // move is the move that leads from this position to childPos
        [[nodiscard]] auto recurse(
            const Position& childPos, const Move move, const bool useNullWindow = false,
            const size_t reduction = 0uz, const size_t extension = 0uz)
            -> AlphaBetaContext
        {
            assert(reduction == 0uz or reduction + 1uz < depthLeft);

            const auto depth = depthLeft + extension;

            return { useNullWindow ? bounds.null_window() : bounds.invert(),
                childPos,
                depth > reduction ? depth - 1uz - reduction : 0uz,
                plyFromRoot + 1uz,
                transTable, interrupter, stats, childPV, killerMoves, history, continuationHistory, pawnTable,
                reset_child_stack_entry(move) };
        }

        // the reduction includes the 1 ply for the null move itself
        [[nodiscard]] auto recurse_null(const size_t reduction) -> AlphaBetaContext
        {
            AlphaBetaContext child {
                bounds.null_window_beta(),
                after_null_move(position),
                depthLeft > reduction ? depthLeft - reduction : 0uz,
                plyFromRoot + 1uz,
                transTable, interrupter, stats, childPV, killerMoves, history, continuationHistory, pawnTable,
                reset_child_stack_entry(Move { })
            };

            // don't allow 2 null moves in a row
            child.nullMoveAllowed = false;

            return child;
        }

        // resets the search stack entry for a child of this node, & returns it.
        // This is called each time a child is searched, so that the child doesn't
        // see anything left over from a node searched earlier at the same ply
        [[nodiscard]] auto reset_child_stack_entry(const Move move) const noexcept -> SearchStackEntry*
        {
            assert(plyFromRoot < MAX_PLY);

            auto* const child = std::next(ss);

            *child = { .move = move, .staticEval = std::nullopt };

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

        ContinuationHistory& continuationHistory; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        PvList& pv; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        PawnTable& pawnTable; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

        // this node's entry in the search stack. The entries for the previous
        // plies, including the sentinels before the root, are std::prev(ss, n)
        SearchStackEntry* ss;

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

    // searches all the root moves, except for the excluded moves, to find the best line.
    // Like the rest of the search, this is fail-soft: if the search fails low, the returned PV
    // is empty & the score is the best of the root moves' scores (an upper bound), & if it fails
    // high, the PV starts with the move that failed high & the score is its score (a lower bound)
    [[nodiscard]] auto search_root_line(
        const size_t         depth,
        const Options&       options,
        const MoveList&      excludedMoves,
        const Position&      position,
        TranspositionTable&  transTable,
        Interrupter&         interrupter,
        KillerMoves&         killerMoves,
        HistoryTable&        history,
        ContinuationHistory& continuationHistory,
        SearchStack&         stack,
        PawnTable&           pawnTable,
        const Callbacks&     callbacks,
        Stats&               stats,
        const Bounds         initialBounds = { })
        -> Result::Line
    {
        Bounds   bounds { initialBounds };
        MoveList pv;

        // the best score of any root move searched so far, which may be below alpha
        std::optional<Score> bestScore;

        const auto lineStartNodes = stats.nodesSearched;

        // in MultiPV mode, only the first line counts for the time management
        if (excludedMoves.empty())
            stats.bestMoveNodes = 0uz;

        auto moveIdx = 0uz;

        for (const auto move : options.movesToSearch) {
            if (std::ranges::find(excludedMoves, move) != excludedMoves.end())
                continue;

            // with an aspiration window, the first move may fail low without raising alpha,
            // but the later moves should still get a null window search first
            const bool isFirstMove = moveIdx == 0uz;

            callbacks.root_move(move, moveIdx++);

            const auto nodesBefore = stats.nodesSearched;

            PvList childPV;

            auto create_context = [depth, move, &transTable, &interrupter, &stats, &childPV, &killerMoves, &history, &continuationHistory, &stack, &pawnTable](const Bounds boundsToUse, const Position& pos) {
                // reset each time, as in AlphaBetaContext::recurse()
                auto* const stackEntry = stack.at(1uz);

                *stackEntry = { .move = move, .staticEval = std::nullopt };

                return AlphaBetaContext {
                    boundsToUse, pos, depth, 1uz, transTable, interrupter, stats, childPV, killerMoves, history, continuationHistory, pawnTable, stackEntry
                };
            };

            // principal variation search: first try searching with a null window
            const auto score = [bounds, &position, move, isFirstMove, make_context = std::move(create_context)] {
                const auto newPos = after_move(position, move);

                if (isFirstMove) {
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

            // an aborted search's score is meaningless, so it mustn't make this the best move
            if (interrupter.was_aborted())
                break;

            if (not bestScore.has_value() or score > *bestScore)
                bestScore = score;

            if (score > bounds.alpha) {
                bounds.alpha = score;

                pv.clear();
                pv.emplace_back(move);

                // in MultiPV mode, only the first line's best move counts
                if (excludedMoves.empty())
                    stats.bestMoveNodes = stats.nodesSearched - nodesBefore;

                childPV.to_movelist(pv);

                killerMoves.store(0uz, move);
            }

            // fail high (only possible with an aspiration window)
            if (bounds.alpha >= bounds.beta)
                break;
        }

        if (excludedMoves.empty())
            stats.firstLineNodes = stats.nodesSearched - lineStartNodes;

        return {
            .score = pv.empty() ? bestScore.value_or(bounds.alpha) : bounds.alpha,
            .pv    = pv
        };
    }

    // aspiration windows: searches the first line with a narrow window around the average score
    // of the previous iterations, & widens the window on the side that failed until the score is
    // inside it. Each fail high searches 1 ply less, since we already know the score is at
    // least as good as we expected, until a fail low resets the depth
    [[nodiscard]] auto search_root_line_aspiration(
        const size_t               depth,
        Options&                   options,
        const Position&            position,
        TranspositionTable&        transTable,
        Interrupter&               interrupter,
        KillerMoves&               killerMoves,
        HistoryTable&              history,
        ContinuationHistory&       continuationHistory,
        SearchStack&               stack,
        PawnTable&                 pawnTable,
        const Callbacks&           callbacks,
        Stats&                     stats,
        const std::optional<Score> avgScore)
        -> Result::Line
    {
        const MoveList excludedMoves;

        // don't use a window if there's no score to center it on, or if it's a mate or so big
        // that the window would be opened all the way anyway
        if (not avgScore.has_value() or avgScore->is_mate()
            or std::abs(static_cast<int>(avgScore->value)) >= ASPIRATION_MAX_SCORE) {
            return search_root_line(
                depth, options, excludedMoves, position, transTable, interrupter, killerMoves, history, continuationHistory, stack, pawnTable, callbacks, stats);
        }

        static constexpr int maxValue = eval::MAX;

        const int center = avgScore->value;

        auto delta = aspiration_delta(center);

        auto alpha = center - delta;
        auto beta  = center + delta;

        auto failHighReduction = 0uz;

        while (true) {
            const Bounds bounds {
                .alpha = Score { static_cast<eval::Value>(alpha) },
                .beta  = Score { static_cast<eval::Value>(beta) }
            };

            auto line = search_root_line(
                depth - failHighReduction, options, excludedMoves, position, transTable, interrupter,
                killerMoves, history, continuationHistory, stack, pawnTable, callbacks, stats, bounds);

            // root_search() decides what to keep from an aborted search
            if (interrupter.was_aborted())
                return line;

            const int score = line.score.value;

            if (line.pv.empty() or line.score <= bounds.alpha) {
                // fail low: beta moves to the middle of the old window, & alpha goes below
                // the score that the search found
                beta  = (alpha + beta) / 2;
                alpha = score - delta;

                failHighReduction = 0uz;
            } else if (line.score >= bounds.beta) {
                // fail high: beta goes above the score that the search found, & the move that
                // failed high is searched first in the re-search
                beta = score + delta;

                if (not line.score.is_mate()) {
                    failHighReduction = std::min({ failHighReduction + 1uz,
                        ASPIRATION_MAX_FAIL_HIGH_REDUCTION,
                        depth - 1uz });
                }

                if (const auto it = std::ranges::find(options.movesToSearch, line.pv.front());
                    it != options.movesToSearch.end()) {
                    std::rotate(options.movesToSearch.begin(), it, std::next(it));
                }
            } else {
                return line;
            }

            // this also opens the window all the way if the search found a mate score
            if (alpha <= -ASPIRATION_MAX_SCORE)
                alpha = -maxValue;

            if (beta >= ASPIRATION_MAX_SCORE)
                beta = maxValue;

            delta += delta / 2;
        }
    }

    [[nodiscard]] auto root_search(
        const size_t               depth,
        Options&                   options,
        const Position&            position,
        TranspositionTable&        transTable,
        Interrupter&               interrupter,
        KillerMoves&               killerMoves,
        HistoryTable&              history,
        ContinuationHistory&       continuationHistory,
        SearchStack&               stack,
        PawnTable&                 pawnTable,
        const Callbacks&           callbacks,
        const std::optional<Move>  prevBestMove,
        const std::optional<Score> windowCenter)
        -> RootSearchResult
    {
        const Timer timer;

        killerMoves.clear();

        // the root has no previous moves, so this doesn't use the continuation history
        detail::order_moves_for_search(
            position, options.movesToSearch, transTable, killerMoves.get(0uz), history,
            continuationHistory, Move { }, Move { });

        // the root position is never stored in the transposition table, so the move ordering
        // can't know about the previous iteration's best move. Search it first explicitly
        if (prevBestMove.has_value()) {
            if (const auto it = std::ranges::find(options.movesToSearch, *prevBestMove);
                it != options.movesToSearch.end()) {
                std::rotate(options.movesToSearch.begin(), it, std::next(it));
            }
        }

        RootSearchResult result;

        result.rootPosition = position;

        // in MultiPV mode, we search the root moves once for each line, each time
        // excluding the first moves of the lines that have already been found
        const auto numLines = std::clamp(options.multiPV, 1uz, options.movesToSearch.size());

        MoveList excludedMoves;

        for (auto lineIdx = 0uz; lineIdx < numLines; ++lineIdx) {
            auto line = lineIdx == 0uz
                          ? search_root_line_aspiration(
                                depth, options, position, transTable, interrupter, killerMoves, history, continuationHistory, stack, pawnTable, callbacks, result.stats, windowCenter)
                          : search_root_line(
                                depth, options, excludedMoves, position, transTable, interrupter, killerMoves, history, continuationHistory, stack, pawnTable, callbacks, result.stats);

            // if the search was aborted, the first line's best move so far was still fully searched,
            // so it's kept (see Context::search()). With an aspiration window, that's only from
            // the search that was aborted, since a search that failed returns an empty PV or a
            // move that failed high, & either way it's searched again
            if (interrupter.was_aborted()) {
                if (lineIdx == 0uz and not line.pv.empty()) {
                    result.pv        = std::move(line.pv);
                    result.bestScore = line.score;
                }

                break;
            }

            if (line.pv.empty())
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

    // the iteration depth from which the soft time limit is scaled by how the search is going.
    // Below this, the iterations are too short for their statistics to mean much
    constexpr auto TM_SCALING_MIN_DEPTH { 7uz };

    // the most consecutive iterations that the best move or score staying the same counts for
    constexpr auto MAX_STABILITY { 4uz };

    // indexed by the number of consecutive iterations that had the same best move
    constexpr std::array<double, MAX_STABILITY + 1uz> BEST_MOVE_STABILITY_SCALES {
        2.43, 1.35, 1.09, 0.88, 0.68
    };

    // indexed by the number of consecutive iterations whose score was within
    // EVAL_STABILITY_MARGIN of the one before
    constexpr std::array<double, MAX_STABILITY + 1uz> EVAL_STABILITY_SCALES {
        1.25, 1.15, 1.00, 0.94, 0.88
    };

    constexpr auto EVAL_STABILITY_MARGIN { 10 };

    // how many iterations in a row the best move & score have stayed the same
    struct Stability final {
        size_t bestMove { 0uz };
        size_t eval { 0uz };

        std::optional<Score> prevScore;

        void update(const std::optional<Move> prevBestMove, const RootSearchResult& result) noexcept
        {
            if (prevBestMove.has_value() and not result.pv.empty() and result.pv.front() == *prevBestMove)
                bestMove = std::min(bestMove + 1uz, MAX_STABILITY);
            else
                bestMove = 0uz;

            if (prevScore.has_value()
                and std::abs(static_cast<int>(result.bestScore) - static_cast<int>(*prevScore)) <= EVAL_STABILITY_MARGIN)
                eval = std::min(eval + 1uz, MAX_STABILITY);
            else
                eval = 0uz;

            prevScore = result.bestScore;
        }
    };

    // When most of the nodes went into searching the best move, the other moves were refuted
    // quickly, so the best move probably won't change & we can stop sooner. When the best move
    // took only a small share of the nodes, the other moves were hard to refute, so it's worth
    // searching longer. The node scale goes from 0.675 (all the nodes) to 2.025 (none of them).
    // We also stop sooner when the best move & score have stayed the same for a few iterations,
    // & search longer when they've just changed
    [[nodiscard]] auto scale_soft_time_limit(
        const milliseconds softLimit, const size_t depth, const Stats& stats, const Stability& stability) noexcept -> milliseconds
    {
        if (depth < TM_SCALING_MIN_DEPTH or stats.firstLineNodes == 0uz)
            return softLimit;

        const auto bestMoveFraction = static_cast<double>(stats.bestMoveNodes)
                                    / static_cast<double>(stats.firstLineNodes);

        const auto scale = (1.5 - bestMoveFraction) * 1.35
                         * BEST_MOVE_STABILITY_SCALES.at(stability.bestMove)
                         * EVAL_STABILITY_SCALES.at(stability.eval);

        return milliseconds {
            static_cast<milliseconds::rep>(static_cast<double>(softLimit.count()) * scale)
        };
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
    history->age();

    // the continuation history isn't aged: it has far more entries than the main history, so
    // each one gets few updates in a search, & halving it every move would throw most of that away

    // the sentinels before the root are never written to, so they keep the null moves & missing
    // static evals that they start with. The root's entry keeps its null move (the game's real
    // previous moves aren't known here, since Position doesn't store them), but gets the root's
    // static eval, so that nodes 2 plies from the root can compare with it.
    SearchStack stack;

    if (not position.is_check())
        stack.at(0uz)->staticEval = eval::evaluate(position, &pawnTable);

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

    // the deepest iteration we'll search. Without this limit, when we're being mated each
    // iteration only searches a handful of nodes, so we'd keep going far past MAX_PLY
    const auto maxDepth = std::min(options.depth, MAX_DEPTH);

    Stability stability;

    std::optional<Score> avgScore;

    while ((options.infinite or pondering.load(memory_order::acquire) or depth <= maxDepth)
           and depth <= MAX_DEPTH) {
        if (interrupter.should_abort(0uz))
            break;

        const auto prevBestMove = result.pv.empty()
                                    ? std::nullopt
                                    : std::optional { result.pv.front() };

        const auto windowCenter = completedDepth >= ASPIRATION_MIN_DEPTH
                                    ? avgScore
                                    : std::nullopt;

        const auto res = root_search(
            depth, options, position, transTable, interrupter, killerMoves, *history, *continuationHistory, stack, pawnTable, callbacks, prevBestMove, windowCenter);

        if (interrupter.was_aborted()) {
            // The previous best move is searched first, so any move that the aborted iteration
            // found was fully searched to this depth & either is that move or beat it. With
            // MultiPV, the other lines would be lost, so the last complete iteration is kept.
            // The reported depth is still the last one that was completed
            if (not res.pv.empty() and options.multiPV == 1uz)
                result = res;

            break;
        }

        result         = res;
        completedDepth = depth;

        stability.update(prevBestMove, result);

        // the aspiration window is centered on the average of the iterations' scores, weighted
        // towards the latest ones, which swings less than the latest score on its own. A mate
        // score resets it, since it doesn't average with anything
        if (avgScore.has_value() and not avgScore->is_mate() and not result.bestScore.is_mate()) {
            avgScore = Score { static_cast<eval::Value>(
                (static_cast<int>(avgScore->value) + static_cast<int>(result.bestScore.value)) / 2) };
        } else {
            avgScore = result.bestScore;
        }

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
                and interrupter.get_search_duration() >= scale_soft_time_limit(*options.softSearchTime, depth, result.stats, stability))
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
        if (depth < maxDepth or options.infinite or pondering.load(memory_order::acquire)) {
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
