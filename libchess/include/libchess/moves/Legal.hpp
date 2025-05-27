/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for generating legal moves.

    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace provides functions for generating legal moves for each piece type
    from a given starting position. These functions rely on other board state, such
    as sets of empty or occupied squares, etc.

    @ingroup moves
 */
namespace chess::moves::legal {

using board::Bitboard;
using board::Square;
using pieces::Color;

// bishop
// queen

/// @ingroup moves
/// @{

/** Calculates all legal pawn pushes.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.
 */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_pushes(
    Bitboard startingPawns, Color color, Bitboard occupiedSquares) noexcept;

/** Calculates all legal pawn double pushes.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.
 */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_double_pushes(
    Bitboard startingPawns, Color color, Bitboard occupiedSquares) noexcept;

/** Calculates all legal pawn captures. */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_captures(
    Bitboard startingPawns, Color color, Bitboard enemyPieces) noexcept;

/** Calculates all legal knight moves. */
[[nodiscard, gnu::const]] constexpr Bitboard knight(
    Bitboard startingKnights, Bitboard friendlyPieces) noexcept;

/** Calculates all legal rook moves, taking blocking pieces into consideration.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.
 */
[[nodiscard, gnu::const]] constexpr Bitboard rook(
    const Square& starting, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

/** Calculates all legal king moves. */
[[nodiscard, gnu::const]] constexpr Bitboard king(
    Bitboard startingKing, Bitboard friendlyPieces) noexcept;

/// @}

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

constexpr Bitboard pawn_pushes(
    const Bitboard startingPawns, const Color color, const Bitboard occupiedSquares) noexcept
{
    return pseudo_legal::pawn_pushes(startingPawns, color) & occupiedSquares.inverse();
}

constexpr Bitboard pawn_double_pushes(
    const Bitboard startingPawns, const Color color, const Bitboard occupiedSquares) noexcept
{
    return pseudo_legal::pawn_double_pushes(startingPawns, color) & occupiedSquares.inverse();
}

constexpr Bitboard pawn_captures(
    const Bitboard startingPawns, const Color color, const Bitboard enemyPieces) noexcept
{
    return pseudo_legal::pawn_attacks(startingPawns, color) & enemyPieces;
}

constexpr Bitboard knight(
    const Bitboard startingKnights, const Bitboard friendlyPieces) noexcept
{
    return pseudo_legal::knight(startingKnights) & friendlyPieces.inverse();
}

constexpr Bitboard king(
    const Bitboard startingKing, const Bitboard friendlyPieces) noexcept
{
    return pseudo_legal::king(startingKing) & friendlyPieces.inverse();
}

namespace detail {

    using board::BitboardIndex;

    enum class RayDirection {
        North,
        East,
        South,
        West
    };

    [[nodiscard, gnu::const]] constexpr bool is_negative(const RayDirection direction) noexcept
    {
        return std::cmp_greater_equal(
            std::to_underlying(direction),
            std::to_underlying(RayDirection::South));
    }

    [[nodiscard, gnu::const]] constexpr Bitboard make_ray(
        const BitboardIndex startPos, const RayDirection direction) noexcept
    {
        static constexpr Bitboard::Integer ONE { 1 };

        switch (direction) {
            case RayDirection::North: {
                static constexpr Bitboard mask { 0x0101010101010100 };

                return mask << startPos;
            }

            case RayDirection::South: {
                static constexpr Bitboard mask { 0x0080808080808080 };

                return mask >> (startPos ^ 63);
            }

            case RayDirection::East: {
                return Bitboard {
                    2 * ((ONE << (startPos | 7)) - (ONE << startPos))
                };
            }

            case RayDirection::West: {
                return Bitboard {
                    (ONE << startPos) - (ONE << (startPos & 56))
                };
            }

            default:
                std::unreachable();
        }
    }

    // returns all squares accessible by a ray attacker in the given direction,
    // stopping at the first blocking piece as indicated by the occupied bitboard
    [[nodiscard, gnu::const]] constexpr Bitboard ray_attacks(
        const BitboardIndex startPos, const RayDirection direction, const Bitboard occupied) noexcept
    {
        auto attacks = make_ray(startPos, direction);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = is_negative(direction) ? blocker.last() : blocker.first();

            attacks ^= make_ray(idx, direction);
        }

        return attacks;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard rook_attacks(
        const BitboardIndex startPos, const Bitboard occupiedSquares) noexcept
    {
        return ray_attacks(startPos, RayDirection::North, occupiedSquares)
             | ray_attacks(startPos, RayDirection::East, occupiedSquares)
             | ray_attacks(startPos, RayDirection::South, occupiedSquares)
             | ray_attacks(startPos, RayDirection::West, occupiedSquares);
    }

} // namespace detail

constexpr Bitboard rook(
    const Square& starting, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    return detail::rook_attacks(starting.index(), occupiedSquares) & friendlyPieces.inverse();
}

} // namespace chess::moves::legal
