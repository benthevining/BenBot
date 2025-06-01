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

/** This namespace contains functions for evaluating positions.
    @ingroup eval
 */
namespace chess::eval {

using game::Position;

/** Returns a numerical score representing the evaluation of the
    give position from the perspective of the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr float evaluate(const Position& position) noexcept;

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

    [[nodiscard, gnu::const]] constexpr float material_score(
        const Pieces& ourPieces, const Pieces& theirPieces) noexcept
    {
        return static_cast<float>(ourPieces.material()) - static_cast<float>(theirPieces.material())
    }

} // namespace detail

constexpr float evaluate(const Position& position) noexcept
{
    using pieces::Color;

    const auto& ourPieces   = position.sideToMove == Color::White ? position.whitePieces : position.blackPieces;
    const auto& theirPieces = position.sideToMove == Color::White ? position.blackPieces : position.whitePieces;

    return detail::material_score(ourPieces, theirPieces);
}

} // namespace chess::eval
