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
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/Colors.hpp>
#include <limits>

/** This namespace contains functions for evaluating positions.
    @ingroup eval
 */
namespace chess::eval {

using game::Position;

/** Floating-point type representing evaluation results.
    @ingroup eval
 */
using Value = double;

/** The maximum possible evaluation score, i.e., if the side to move
    has mate-in-1.
    @ingroup eval
 */
static constexpr auto MAX = std::numeric_limits<Value>::max();

/** The minimum possible evaluation score, i.e., if the side to move
    has gotten checkmated.
    @ingroup eval
 */
static constexpr auto MIN = std::numeric_limits<Value>::min();

/** Returns a numerical score representing the evaluation of the
    give position from the perspective of the side to move.
    Higher numbers are better for the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr Value evaluate(const Position& position) noexcept;

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

    [[nodiscard, gnu::const]] constexpr Value material_score(
        const Pieces& ourPieces, const Pieces& theirPieces) noexcept
    {
        return static_cast<Value>(ourPieces.material())
             - static_cast<Value>(theirPieces.material());
    }

} // namespace detail

constexpr Value evaluate(const Position& position) noexcept
{
    using pieces::Color;

    if (! moves::any_legal_moves(position)) {
        if (position.is_check())
            return MIN; // we got mated

        return 0.; // stalemate
    }

    const auto& ourPieces   = position.sideToMove == Color::White ? position.whitePieces : position.blackPieces;
    const auto& theirPieces = position.sideToMove == Color::White ? position.blackPieces : position.whitePieces;

    return detail::material_score(ourPieces, theirPieces);
}

} // namespace chess::eval
