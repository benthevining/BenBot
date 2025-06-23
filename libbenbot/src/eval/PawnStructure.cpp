/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include "PawnStructure.hpp" // NOLINT(build/include_subdir)
#include <array>
#include <cmath>                        // IWYU pragma: keep - for std::abs()
#include <libchess/board/Distances.hpp> // IWYU pragma: keep - for chebyshev_distance()
#include <libchess/board/Fills.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>

namespace chess::eval::detail {

namespace {

    using pieces::Color;

    template <Color Side>
    [[nodiscard, gnu::const]] int score_side_passed_pawns(const Position& position)
    {
        using board::Rank;

        static constexpr auto OtherSide = pieces::other_side<Side>();

        static constexpr auto promotionRank = Side == Color::White ? Rank::Eight : Rank::One;

        static constexpr auto ROOK_BEHIND_BONUS           = 25;
        static constexpr auto KING_ESCORT_BONUS           = 2;
        static constexpr auto ENEMY_KING_BLOCKING_PENALTY = 50;

        const auto passers = position.get_passed_pawns<Side>();

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

            const auto squaresFromPromoting = std::abs(
                static_cast<int>(square.rank) - static_cast<int>(promotionRank));

            static constexpr std::array bonuses {
                10000, 100, 85, 70, 60, 50, 35
            };

            score += bonuses.at(squaresFromPromoting);

            // penalty for enemy king on promotion square
            // this is intended to help the engine reduce the draw rate in king/pawn endgames,
            // by seeing that if the enemy king can blockade the promotion square, then we're
            // less likely to be able to force promotion
            if (enemyKing == board::Square { .file = square.file, .rank = promotionRank })
                score -= ENEMY_KING_BLOCKING_PENALTY;
        }

        return score;
    }

    [[nodiscard, gnu::const]] int score_passed_pawns(const Position& position)
    {
        const auto whiteScore = score_side_passed_pawns<Color::White>(position);
        const auto blackScore = score_side_passed_pawns<Color::Black>(position);

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return ourScore - theirScore;
    }

    template <Color Side>
    [[nodiscard, gnu::const]] int score_side_isolated_pawns(const Position& position) noexcept
    {
        auto score { 0 };

        const auto ourPawns = position.pieces_for<Side>().pawns;

        for (const auto pawn : ourPawns.subboards()) {
            const auto mask = board::fills::file(
                moves::patterns::pawn_attacks<Side>(pawn));

            if ((mask & ourPawns).none())
                score -= 20;
        }

        return score;
    }

    [[nodiscard, gnu::const]] int score_isolated_pawns(const Position& position) noexcept
    {
        const auto whiteScore = score_side_isolated_pawns<Color::White>(position);
        const auto blackScore = score_side_isolated_pawns<Color::Black>(position);

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return ourScore - theirScore;
    }

} // namespace

[[nodiscard, gnu::const]] int score_pawn_structure(const Position& position)
{
    return score_passed_pawns(position)
         + score_isolated_pawns(position);
}

} // namespace chess::eval::detail
