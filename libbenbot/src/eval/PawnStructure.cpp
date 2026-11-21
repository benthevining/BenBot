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
#include <array>
#include <cmath> // IWYU pragma: keep - for std::abs()
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
    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_passed_pawns(
        const Position& position, const Bitboard passers)
        -> int
    {
        static constexpr auto promotionRank = board::back_rank_for(chess::pieces::other_side<Side>());

        const auto ourPawns = position.pieces_for<Side>().pawns;

        auto score { 0 };

        for (const auto pawn : passers.subboards()) {
            const auto square = board::Square::from_index(pawn.first());

            const auto squaresFromPromoting = std::abs(
                static_cast<int>(square.rank) - static_cast<int>(promotionRank));

            static constexpr std::array bonuses {
                10000, 100, 85, 70, 60, 50, 35
            };

            score += bonuses.at(static_cast<size_t>(squaresFromPromoting));

            namespace shifts = board::shifts;

            // bonus for being protected by a friendly pawn

            const auto protectorMask
                = shifts::pawn_inv_capture_east<Side>(pawn)
                | shifts::pawn_inv_capture_west<Side>(pawn);

            for (const auto protector : (ourPawns & protectorMask).subboards()) {
                if ((protector & passers).any()) // protected passed pawns
                    score += 15;
                else
                    score += 10; // protected by a non-passed pawn
            }
        }

        return score;
    }

    // the passed pawn terms that depend on the rooks & kings, so these can't be cached by pawn hash
    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_passer_piece_terms(
        const Position& position, const Bitboard passers) noexcept
        -> int
    {
        static constexpr auto OtherSide = chess::pieces::other_side<Side>();

        static constexpr auto promotionRank = board::back_rank_for(OtherSide);

        static constexpr auto ROOK_BEHIND_BONUS           = 25;
        static constexpr auto KING_ESCORT_BONUS           = 2;
        static constexpr auto ENEMY_KING_BLOCKING_PENALTY = 50;

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OtherSide>();

        auto score { 0 };

        const auto rooks = ourPieces.rooks;
        const auto king  = ourPieces.get_king_location();

        const auto enemyKing = theirPieces.get_king_location();

        for (const auto pawn : passers.subboards()) {
            const auto mask = board::fills::pawn_rear<Side>(pawn);

            // bonus for friendly rook behind the pawn, penalty for enemy rook behind the pawn
            if ((mask & rooks).any())
                score += ROOK_BEHIND_BONUS;
            else if ((mask & theirPieces.rooks).any())
                score -= ROOK_BEHIND_BONUS;

            const auto square = board::Square::from_index(pawn.first());

            // bonus for king closer to passed pawn
            {
                static constexpr auto MAX_DIST = 7uz;

                const auto kingDist = chebyshev_distance(king, square);

                score += static_cast<int>(MAX_DIST - kingDist) * KING_ESCORT_BONUS;
            }

            // penalty for enemy king on promotion square
            // this is intended to help the engine reduce the draw rate in king/pawn endgames,
            // by seeing that if the enemy king can blockade the promotion square, then we're
            // less likely to be able to force promotion
            if (enemyKing == board::Square { .file = square.file, .rank = promotionRank })
                score -= ENEMY_KING_BLOCKING_PENALTY;
        }

        return score;
    }

    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_isolated_pawns(const Position& position) noexcept -> int
    {
        auto score { 0 };

        const auto ourPawns = position.pieces_for<Side>().pawns;

        for (const auto pawn : ourPawns.subboards()) {
            const auto mask = board::fills::file(
                chess::moves::patterns::pawn_attacks<Side>(pawn));

            if ((mask & ourPawns).none())
                score -= 20;
        }

        return score;
    }

    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_doubled_pawns(const Position& position) noexcept -> int
    {
        auto score { 0 };

        const auto pawns = position.pieces_for<Side>().pawns;

        for (const auto file : magic_enum::enum_values<board::File>()) {
            const auto pawnsOnFile = (pawns & board::masks::files::get(file)).count();

            if (pawnsOnFile > 1uz)
                score -= 10;
        }

        return score;
    }

    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_backward_pawns(const Position& position) noexcept -> int
    {
        return static_cast<int>(position.get_backward_pawns<Side>().count()) * -20;
    }

    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_pawn_chains(const Position& position) noexcept -> int
    {
        // for each pawn, we award a bonus for each square it attacks that has a friendly pawn on it
        // this has the effect of awarding a larger bonus for larger pawn chains

        auto score { 0 };

        const auto pawns = position.pieces_for<Side>().pawns;

        for (const auto pawn : pawns.subboards()) {
            const auto attacks = chess::moves::patterns::pawn_attacks<Side>(pawn);

            const auto defended = static_cast<int>((pawns & attacks).count());

            score += (defended * 10);
        }

        return score;
    }

    template <Color Side>
    [[nodiscard, gnu::pure]] auto score_side_pawns(
        const Position& position, const Bitboard passers)
        -> int
    {
        return score_side_passed_pawns<Side>(position, passers)
             + score_side_isolated_pawns<Side>(position)
             + score_side_doubled_pawns<Side>(position)
             + score_side_backward_pawns<Side>(position)
             + score_side_pawn_chains<Side>(position);
    }

} // namespace

auto score_pawns_only(const Position& position) -> PawnTerms
{
    const auto whitePassers = position.get_passed_pawns<Color::White>();
    const auto blackPassers = position.get_passed_pawns<Color::Black>();

    return {
        .whiteScore = score_side_pawns<Color::White>(position, whitePassers)
                    - score_side_pawns<Color::Black>(position, blackPassers),
        .passers    = whitePassers | blackPassers
    };
}

auto score_passer_piece_terms(
    const Position& position, const Bitboard passers) noexcept
    -> int
{
    return score_side_passer_piece_terms<Color::White>(position, passers & position.whitePieces.pawns)
         - score_side_passer_piece_terms<Color::Black>(position, passers & position.blackPieces.pawns);
}

auto score_pawn_structure(const Position& position) -> int
{
    const auto pawnTerms = score_pawns_only(position);

    const auto whiteScore = pawnTerms.whiteScore + score_passer_piece_terms(position, pawnTerms.passers);

    return position.is_white_to_move() ? whiteScore : -whiteScore;
}

} // namespace ben_bot::eval::detail
