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

using std::size_t;

/** A debugging function that walks the entire move tree and returns the number of visited leaf nodes.

    @ingroup moves
 */
[[nodiscard]] constexpr size_t perft(size_t depth);

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

    [[nodiscard]] constexpr size_t perft_internal( // NOLINT(misc-no-recursion)
        const game::Position& position, const size_t depth)
    {
        if (depth == 0uz)
            return 1uz;

        auto nodes = 0uz;

        for (const auto& move : generate_legal_moves(position)) {
            game::Position newPosition { position };

            newPosition.make_move(move);

            nodes += perft_internal(newPosition, depth - 1uz);
        }

        return nodes;
    }

} // namespace detail

constexpr size_t perft(const size_t depth)
{
    static constexpr game::Position position;

    return detail::perft_internal(position, depth);
}

} // namespace chess::moves
