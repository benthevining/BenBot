/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides a perft function for testing move generation.
    @ingroup moves
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>

namespace chess::moves {

using game::Position;
using std::size_t;

/** A debugging function that walks the entire move tree and returns the number of visited leaf nodes.

    @ingroup moves
 */
[[nodiscard]] constexpr size_t perft(size_t depth, const Position& startingPosition = {});

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

constexpr size_t perft(const size_t depth, const Position& startingPosition) // NOLINT(misc-no-recursion)
{
    if (depth == 0uz)
        return 1uz;

    auto nodes = 0uz;

    for (const auto& move : generate_legal_moves(startingPosition)) {
        Position newPosition { startingPosition };

        newPosition.make_move(move);

        nodes += perft(depth - 1uz, newPosition);
    }

    return nodes;
}

} // namespace chess::moves
