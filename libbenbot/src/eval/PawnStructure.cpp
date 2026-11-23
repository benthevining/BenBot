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

#include "eval/PawnStructure.hpp"
#include <cassert>
#include <cmath> // IWYU pragma: keep - for std::abs()
#include <libbenbot/data-structures/PawnTable.hpp>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libbenbot/eval/Trace.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp> // IWYU pragma: keep - for chebyshev_distance()
#include <libchess/board/File.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>

namespace ben_bot::eval::detail {

using chess::board::Bitboard;
using chess::pieces::Color;

namespace {

    namespace board = chess::board;

    // the passed pawn terms that only depend on the pawns
    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_passed_pawns(
        const Position& position, const Bitboard passers, Tracer& tracer)
        -> TaperedScore
    {
        static constexpr auto promotionRank = board::back_rank_for(chess::pieces::other_side<Side>());

        const auto ourPawns = position.pieces_for<Side>().pawns;

        TaperedScore score;

        for (const auto pawn : passers.subboards()) {
            const auto square = board::Square::from_index(pawn.first());

            const auto squaresFromPromoting = std::abs(
                static_cast<int>(square.rank) - static_cast<int>(promotionRank));

            // a pawn can't be on its promotion rank
            assert(squaresFromPromoting > 0);

            score += DEFAULT_WEIGHTS.get(Term::PassedPawn, static_cast<size_t>(squaresFromPromoting - 1));
            tracer.add(Term::PassedPawn, Side, 1, static_cast<size_t>(squaresFromPromoting - 1));

            namespace shifts = board::shifts;

            // bonus for being protected by a friendly pawn

            const auto protectorMask
                = shifts::pawn_inv_capture_east<Side>(pawn)
                | shifts::pawn_inv_capture_west<Side>(pawn);

            for (const auto protector : (ourPawns & protectorMask).subboards()) {
                if ((protector & passers).any()) { // protected passed pawns
                    score += DEFAULT_WEIGHTS.get(Term::PasserProtectedByPasser);
                    tracer.add(Term::PasserProtectedByPasser, Side, 1);
                } else { // protected by a non-passed pawn
                    score += DEFAULT_WEIGHTS.get(Term::PasserProtectedByPawn);
                    tracer.add(Term::PasserProtectedByPawn, Side, 1);
                }
            }
        }

        return score;
    }

    // the passed pawn terms that depend on the rooks & kings, so these can't be cached by pawn hash
    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_passer_piece_terms(
        const Position& position, const Bitboard passers, Tracer& tracer) noexcept
        -> TaperedScore
    {
        static constexpr auto OtherSide = chess::pieces::other_side<Side>();

        static constexpr auto promotionRank = board::back_rank_for(OtherSide);

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OtherSide>();

        TaperedScore score;

        const auto rooks = ourPieces.rooks;
        const auto king  = ourPieces.get_king_location();

        const auto enemyKing = theirPieces.get_king_location();

        for (const auto pawn : passers.subboards()) {
            const auto mask = board::fills::pawn_rear<Side>(pawn);

            // bonus for friendly rook behind the pawn, penalty for enemy rook behind the pawn
            if ((mask & rooks).any()) {
                score += DEFAULT_WEIGHTS.get(Term::RookBehindPasser);
                tracer.add(Term::RookBehindPasser, Side, 1);
            } else if ((mask & theirPieces.rooks).any()) {
                score -= DEFAULT_WEIGHTS.get(Term::RookBehindPasser);
                tracer.add(Term::RookBehindPasser, Side, -1);
            }

            const auto square = board::Square::from_index(pawn.first());

            // bonus for king closer to passed pawn
            {
                static constexpr auto MAX_DIST = 7uz;

                const auto kingDist = static_cast<int>(MAX_DIST - chebyshev_distance(king, square));

                score += DEFAULT_WEIGHTS.get(Term::KingEscortsPasser) * kingDist;
                tracer.add(Term::KingEscortsPasser, Side, kingDist);
            }

            // penalty for enemy king on promotion square
            // this is intended to help the engine reduce the draw rate in king/pawn endgames,
            // by seeing that if the enemy king can blockade the promotion square, then we're
            // less likely to be able to force promotion
            if (enemyKing == board::Square { .file = square.file, .rank = promotionRank }) {
                score += DEFAULT_WEIGHTS.get(Term::EnemyKingBlocksPasser);
                tracer.add(Term::EnemyKingBlocksPasser, Side, 1);
            }
        }

        return score;
    }

    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_isolated_pawns(const Position& position, Tracer& tracer) noexcept -> TaperedScore
    {
        TaperedScore score;

        const auto ourPawns = position.pieces_for<Side>().pawns;

        for (const auto pawn : ourPawns.subboards()) {
            const auto mask = board::fills::file(
                chess::moves::patterns::pawn_attacks<Side>(pawn));

            if ((mask & ourPawns).none()) {
                score += DEFAULT_WEIGHTS.get(Term::IsolatedPawn);
                tracer.add(Term::IsolatedPawn, Side, 1);
            }
        }

        return score;
    }

    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_doubled_pawns(const Position& position, Tracer& tracer) noexcept -> TaperedScore
    {
        TaperedScore score;

        const auto pawns = position.pieces_for<Side>().pawns;

        for (const auto file : magic_enum::enum_values<board::File>()) {
            const auto pawnsOnFile = (pawns & board::masks::files::get(file)).count();

            if (pawnsOnFile > 1uz) {
                score += DEFAULT_WEIGHTS.get(Term::DoubledPawn);
                tracer.add(Term::DoubledPawn, Side, 1);
            }
        }

        return score;
    }

    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_backward_pawns(const Position& position, Tracer& tracer) noexcept -> TaperedScore
    {
        const auto count = static_cast<int>(position.get_backward_pawns<Side>().count());

        tracer.add(Term::BackwardPawn, Side, count);

        return DEFAULT_WEIGHTS.get(Term::BackwardPawn) * count;
    }

    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_pawn_chains(const Position& position, Tracer& tracer) noexcept -> TaperedScore
    {
        // for each pawn, we award a bonus for each square it attacks that has a friendly pawn on it
        // this has the effect of awarding a larger bonus for larger pawn chains

        TaperedScore score;

        const auto pawns = position.pieces_for<Side>().pawns;

        for (const auto pawn : pawns.subboards()) {
            const auto attacks = chess::moves::patterns::pawn_attacks<Side>(pawn);

            const auto defended = static_cast<int>((pawns & attacks).count());

            score += DEFAULT_WEIGHTS.get(Term::PawnChain) * defended;
            tracer.add(Term::PawnChain, Side, defended);
        }

        return score;
    }

    template <Color Side, typename Tracer>
    [[nodiscard]] auto score_side_pawns(
        const Position& position, const Bitboard passers, Tracer& tracer)
        -> TaperedScore
    {
        return score_side_passed_pawns<Side>(position, passers, tracer)
             + score_side_isolated_pawns<Side>(position, tracer)
             + score_side_doubled_pawns<Side>(position, tracer)
             + score_side_backward_pawns<Side>(position, tracer)
             + score_side_pawn_chains<Side>(position, tracer);
    }

} // namespace

template <typename Tracer>
auto score_pawns_only(const Position& position, Tracer& tracer) -> PawnTerms
{
    const auto whitePassers = position.get_passed_pawns<Color::White>();
    const auto blackPassers = position.get_passed_pawns<Color::Black>();

    return {
        .whiteScore = score_side_pawns<Color::White>(position, whitePassers, tracer)
                    - score_side_pawns<Color::Black>(position, blackPassers, tracer),
        .passers    = whitePassers | blackPassers
    };
}

template <typename Tracer>
auto score_passer_piece_terms(
    const Position& position, const Bitboard passers, Tracer& tracer) noexcept
    -> TaperedScore
{
    return score_side_passer_piece_terms<Color::White>(position, passers & position.whitePieces.pawns, tracer)
         - score_side_passer_piece_terms<Color::Black>(position, passers & position.blackPieces.pawns, tracer);
}

template <typename Tracer>
auto score_pawn_structure(const Position& position, PawnTable* pawnTable, Tracer& tracer) -> TaperedScore
{
    // a tracer must see every pawn term, so a traced eval never reads or writes the pawn table
    if constexpr (Tracer::ENABLED)
        pawnTable = nullptr;

    const auto pawnTerms = [&position, pawnTable, &tracer] {
        if (pawnTable == nullptr)
            return score_pawns_only(position, tracer);

        auto& entry = pawnTable->probe(position.pawnHash);

        if (entry.key != position.pawnHash) {
            const auto terms = score_pawns_only(position, tracer);

            entry = PawnEntry {
                .key     = position.pawnHash,
                .passers = terms.passers,
                .score   = terms.whiteScore
            };

            return terms;
        }

        return PawnTerms {
            .whiteScore = entry.score,
            .passers    = entry.passers
        };
    }();

    // these terms depend on the rooks & kings, so they're never cached
    const auto whiteScore = pawnTerms.whiteScore + score_passer_piece_terms(position, pawnTerms.passers, tracer);

    return position.is_white_to_move() ? whiteScore : -whiteScore;
}

template auto score_pawn_structure(const Position&, PawnTable*, NullTracer&) -> TaperedScore;
template auto score_pawn_structure(const Position&, PawnTable*, CoeffTracer&) -> TaperedScore;

} // namespace ben_bot::eval::detail
