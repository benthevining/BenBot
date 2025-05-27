/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for generating pseudo-legal moves.

    @ingroup moves
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <utility>

/** This namespace provides functions for generating pseudo-legal moves for each piece
    type from a given starting position. These functions rely on other board state,
    such as sets of empty or occupied squares, etc. The generated moves are pseudo-legal,
    not strictly legal, because they do not consider whether the king would be left in check.
    The returned move sets do not include the starting square.

    @ingroup moves
 */
namespace chess::moves::pseudo_legal {

using board::Bitboard;
using board::Square;
using pieces::Color;

/// @ingroup moves
/// @{

/** Calculates all pseudo-legal pawn pushes.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.
 */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_pushes(
    Bitboard startingPawns, Color color, Bitboard occupiedSquares) noexcept;

/** Calculates all pseudo-legal pawn double pushes.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.
 */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_double_pushes(
    Bitboard startingPawns, Color color, Bitboard occupiedSquares) noexcept;

/** Calculates all pseudo-legal pawn captures.
    The returned bitboard has 1 bits set where each pawn would land after making a capture.
 */
[[nodiscard, gnu::const]] constexpr Bitboard pawn_captures(
    Bitboard startingPawns, Color color, Bitboard enemyPieces) noexcept;

/** Calculates all pseudo-legal knight moves. */
[[nodiscard, gnu::const]] constexpr Bitboard knight(
    Bitboard startingKnights, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal bishop moves.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr Bitboard bishop(
    const Square& starting, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal rook moves, taking blocking pieces into consideration.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr Bitboard rook(
    const Square& starting, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal queen moves, taking blocking pieces into consideration.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr Bitboard queen(
    const Square& starting, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal king moves. */
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
    return patterns::pawn_pushes(startingPawns, color) & occupiedSquares.inverse();
}

constexpr Bitboard pawn_double_pushes(
    const Bitboard startingPawns, const Color color, const Bitboard occupiedSquares) noexcept
{
    namespace rank_masks = board::masks::ranks;

    const auto moves = patterns::pawn_double_pushes(startingPawns, color) & occupiedSquares.inverse();

    // Need to filter out any pushes that would jump over a piece on the third/sixth rank
    const auto rankMask = color == Color::White ? rank_masks::three() : rank_masks::six();

    const auto blockers = occupiedSquares & rankMask;
    const auto fileMask = board::fills::file(blockers);

    return moves & fileMask.inverse();
}

constexpr Bitboard pawn_captures(
    const Bitboard startingPawns, const Color color, const Bitboard enemyPieces) noexcept
{
    return patterns::pawn_attacks(startingPawns, color) & enemyPieces;
}

constexpr Bitboard knight(
    const Bitboard startingKnights, const Bitboard friendlyPieces) noexcept
{
    return patterns::knight(startingKnights) & friendlyPieces.inverse();
}

constexpr Bitboard king(
    const Bitboard startingKing, const Bitboard friendlyPieces) noexcept
{
    return patterns::king(startingKing) & friendlyPieces.inverse();
}

namespace detail {

    using board::BitboardIndex;
    using board::Rank;

    /* Ray directions are as follows:

      northwest    north   northeast
              +7    +8    +9
                  \  |  /
      west    -1 <-  0 -> +1    east
                  /  |  \
              -9    -8    -7
      southwest    south   southeast

      Positive & negative rays are handled individually to determine blockers along that ray.
     */

    enum class RayDirection : std::uint_least8_t {
        // positive rays
        North,
        East,
        NorthEast,
        NorthWest,

        // negative rays
        South,
        West,
        SouthEast,
        SouthWest
    };

    [[nodiscard, gnu::const]] constexpr bool is_negative(const RayDirection direction) noexcept
    {
        return std::cmp_greater_equal(
            std::to_underlying(direction),
            std::to_underlying(RayDirection::South));
    }

    // Generates all squares on the given ray starting from the given square,
    // traveling in the given direction. The starting square is not included
    // in the ray.
    [[nodiscard, gnu::const]] constexpr Bitboard make_ray(
        const Square& startPos, const RayDirection direction) noexcept
    {
        namespace rank_masks = board::masks::ranks;

        static constexpr Bitboard::Integer ONE { 1 };

        switch (direction) {
            case RayDirection::North: {
                static constexpr Bitboard mask { 0x0101010101010100 };

                return mask << startPos.index();
            }

            case RayDirection::South: {
                static constexpr Bitboard mask { 0x0080808080808080 };

                return mask >> (startPos.index() ^ 63);
            }

            case RayDirection::East: {
                const auto index = startPos.index();

                return Bitboard { 2 * ((ONE << (index | 7)) - (ONE << index)) };
            }

            case RayDirection::West: {
                const auto index = startPos.index();

                return Bitboard { (ONE << index) - (ONE << (index & 56)) };
            }

            case RayDirection::NorthEast: {
                const auto diag = board::masks::diagonal(startPos);
                const auto mask = board::fills::south(rank_masks::get(startPos.rank));

                return diag & mask.inverse();
            }

            case RayDirection::NorthWest: {
                const auto diag = board::masks::antidiagonal(startPos);
                const auto mask = board::fills::south(rank_masks::get(startPos.rank));

                return diag & mask.inverse();
            }

            case RayDirection::SouthEast: {
                const auto diag = board::masks::antidiagonal(startPos);
                const auto mask = board::fills::north(rank_masks::get(startPos.rank));

                return diag & mask.inverse();
            }

            case RayDirection::SouthWest: {
                const auto diag = board::masks::diagonal(startPos);
                const auto mask = board::fills::north(rank_masks::get(startPos.rank));

                return diag & mask.inverse();
            }

            default:
                std::unreachable();
        }
    }

    // Returns all squares accessible by a ray attacker in the given direction,
    // stopping at the first blocking piece as indicated by the occupied bitboard.
    // This function does not prune squares occupied by friendly pieces (it considers
    // them possible captures), so those squares still need to be pruned.
    [[nodiscard, gnu::const]] constexpr Bitboard ray_attacks(
        const Square& startPos, const RayDirection direction, const Bitboard occupied) noexcept
    {
        auto attacks = make_ray(startPos, direction);

        const auto blocker = attacks & occupied;

        if (blocker.any()) {
            const auto idx = is_negative(direction) ? blocker.last() : blocker.first();

            attacks ^= make_ray(Square::from_index(idx), direction);
        }

        return attacks;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard rook_attacks(
        const Square& startPos, const Bitboard occupiedSquares) noexcept
    {
        return ray_attacks(startPos, RayDirection::North, occupiedSquares)
             | ray_attacks(startPos, RayDirection::East, occupiedSquares)
             | ray_attacks(startPos, RayDirection::South, occupiedSquares)
             | ray_attacks(startPos, RayDirection::West, occupiedSquares);
    }

    [[nodiscard, gnu::const]] constexpr Bitboard bishop_attacks(
        const Square& startPos, const Bitboard occupiedSquares) noexcept
    {
        return ray_attacks(startPos, RayDirection::NorthEast, occupiedSquares)
             | ray_attacks(startPos, RayDirection::NorthWest, occupiedSquares)
             | ray_attacks(startPos, RayDirection::SouthEast, occupiedSquares)
             | ray_attacks(startPos, RayDirection::SouthWest, occupiedSquares);
    }

} // namespace detail

constexpr Bitboard rook(
    const Square& starting, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    return detail::rook_attacks(starting, occupiedSquares) & friendlyPieces.inverse();
}

constexpr Bitboard bishop(
    const Square& starting, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    return detail::bishop_attacks(starting, occupiedSquares) & friendlyPieces.inverse();
}

constexpr Bitboard queen(
    const Square& starting, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    const auto attacks
        = detail::rook_attacks(starting, occupiedSquares)
        | detail::bishop_attacks(starting, friendlyPieces);

    return attacks & friendlyPieces.inverse();
}

} // namespace chess::moves::pseudo_legal
