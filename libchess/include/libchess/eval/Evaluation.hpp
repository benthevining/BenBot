/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup eval Evaluation
    Functions for evaluating positions.
 */

/** @file
    This file provides the top-level evaluation function.
    @ingroup eval
 */

#pragma once

#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>

/** This namespace contains functions for evaluating positions.
    @ingroup eval
 */
namespace chess::eval {

using game::Position;

/** Returns a numerical score representing the evaluation of the
    give position from the perspective of the side to move.
    Higher numbers are better for the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr double evaluate(const Position& position) noexcept;

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

namespace detail {

    using board::Pieces;

    static constexpr auto NEUTRAL = 0.5;

    // Returns a [0, 1] score where 0 is lone king vs 9 queens, 2 rooks, 2 bishops & 2 knights,
    // and 1 is all those pieces vs lone king
    [[nodiscard, gnu::const]] constexpr double material_score(
        const Pieces& ourPieces, const Pieces& theirPieces) noexcept
    {
        const auto diff = static_cast<double>(ourPieces.material()) - static_cast<double>(theirPieces.material());

        return diff / static_cast<double>(pieces::values::MAX_POSSIBLE_MATERIAL);
    }

    // Returns a [0, 1] score representing the bonus (if any) for having the bishop pair
    [[nodiscard, gnu::const]] constexpr double bishop_pair_score(
        const Pieces& ourPieces, const Pieces& theirPieces) noexcept
    {
        const bool weHave   = ourPieces.has_bishop_pair();
        const bool theyHave = theirPieces.has_bishop_pair();

        if (weHave == theyHave)
            return NEUTRAL;

        // Returns number of pawns left as a 0-1 normalized value
        const auto pawnsLeft = [totalPawns = ourPieces.pawns.count() + theirPieces.pawns.count()] {
            static constexpr auto MAX_PAWNS = 16uz;

            return static_cast<double>(totalPawns) / static_cast<double>(MAX_PAWNS);
        }();

        // 0 when all pawns left, 1 when no pawns left
        const auto pcntPawnsGone = 1. - pawnsLeft;

        // range is now [0, 0.5]
        const auto bishopPairBonus = pcntPawnsGone * 0.5;

        if (weHave)
            return NEUTRAL + bishopPairBonus;

        return NEUTRAL - bishopPairBonus;
    }

} // namespace detail

constexpr double evaluate(const Position& position) noexcept
{
    using pieces::Color;

    const auto& ourPieces   = position.sideToMove == Color::White ? position.whitePieces : position.blackPieces;
    const auto& theirPieces = position.sideToMove == Color::White ? position.blackPieces : position.whitePieces;

    return detail::material_score(ourPieces, theirPieces)
         + detail::bishop_pair_score(ourPieces, theirPieces);
}

} // namespace chess::eval
