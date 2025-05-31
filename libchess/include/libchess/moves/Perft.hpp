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

/** This struct contains information calculated by the ``perft()`` debugging function.

    @ingroup moves
    @see perft()
 */
struct PerftResult final {
    /** The total number of nodes visited by this perft() run. */
    size_t nodes { 0uz };

    /** The number of child nodes that resulted in captures. */
    size_t captures { 0uz };

    /** The number of child nodes that resulted in en passant captures. */
    size_t enPassantCaptures { 0uz };

    /** The number of child nodes that resulted in castling. */
    size_t castles { 0uz };

    /** The number of child nodes that resulted in promotions. */
    size_t promotions { 0uz };

    /** The number of child nodes that resulted in checks. */
    size_t checks { 0uz };

    /** The number of child nodes that resulted in checkmates. */
    size_t checkmates { 0uz };

    /** The number of child nodes that resulted in stalemates. */
    size_t stalemates { 0uz };

    /** Adds the results from a child node. */
    constexpr PerftResult& operator+=(const PerftResult& rhs) noexcept;
};

/** A debugging function that walks the entire move tree and returns the number of visited leaf nodes.

    @ingroup moves
 */
[[nodiscard]] constexpr PerftResult perft(size_t depth, const Position& startingPosition = {});

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

constexpr PerftResult& PerftResult::operator+=(const PerftResult& rhs) noexcept
{
    nodes += rhs.nodes;
    captures += rhs.captures;
    enPassantCaptures += rhs.enPassantCaptures;
    castles += rhs.castles;
    promotions += rhs.promotions;
    checks += rhs.checks;
    checkmates += rhs.checkmates;
    stalemates += rhs.stalemates;

    return *this;
}

constexpr PerftResult perft(const size_t depth, const Position& startingPosition) // NOLINT(misc-no-recursion)
{
    if (depth == 0uz)
        return { .nodes = 1uz };

    PerftResult result;

    for (const auto& move : generate(startingPosition)) {
        if (startingPosition.is_capture(move)) {
            ++result.captures;

            if (startingPosition.is_en_passant(move))
                ++result.enPassantCaptures;
        }

        if (move.is_castling())
            ++result.castles;

        if (move.promotedType.has_value())
            ++result.promotions;

        Position newPosition { startingPosition };

        newPosition.make_move(move);

        const bool isCheck = newPosition.is_check();

        if (isCheck)
            ++result.checks;

        if (! any_legal_moves(newPosition)) {
            if (isCheck)
                ++result.checkmates;
            else
                ++result.stalemates;
        }

        result += perft(depth - 1uz, newPosition);
    }

    return result;
}

} // namespace chess::moves
