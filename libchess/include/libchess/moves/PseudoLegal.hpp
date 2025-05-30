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

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>

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
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_pushes(
    Bitboard startingPawns, Bitboard occupiedSquares) noexcept;

/** Calculates all pseudo-legal pawn double pushes.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_double_pushes(
    Bitboard startingPawns, Bitboard occupiedSquares) noexcept;

/** Calculates all pseudo-legal pawn captures.
    The returned bitboard has 1 bits set where each pawn would land after making a capture.
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_captures(
    Bitboard startingPawns, Bitboard enemyPieces) noexcept;

/** Calculates all pseudo-legal knight moves.
    This function can calculate moves for any number of knights.
 */
[[nodiscard, gnu::const]] constexpr Bitboard knight(
    Bitboard startingKnights, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal bishop moves.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr Bitboard bishop(
    Bitboard startingBishops, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal rook moves, taking blocking pieces into consideration.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr Bitboard rook(
    Bitboard startingRooks, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

/** Calculates all pseudo-legal queen moves, taking blocking pieces into consideration.
    ``occupiedSquares`` should be the union of all squares occupied by pieces of either color.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr Bitboard queen(
    Bitboard startingQueens, Bitboard occupiedSquares, Bitboard friendlyPieces) noexcept;

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

template <Color Side>
constexpr Bitboard pawn_pushes(
    const Bitboard startingPawns, const Bitboard occupiedSquares) noexcept
{
    return patterns::pawn_pushes<Side>(startingPawns) & occupiedSquares.inverse();
}

template <Color Side>
constexpr Bitboard pawn_double_pushes(
    const Bitboard startingPawns, const Bitboard occupiedSquares) noexcept
{
    const auto moves = patterns::pawn_double_pushes<Side>(startingPawns) & occupiedSquares.inverse();

    // Need to filter out any pushes that would jump over a piece on the third/sixth rank
    static constexpr auto rankMask = Side == Color::White
                                       ? board::masks::ranks::THREE
                                       : board::masks::ranks::SIX;

    const auto fileMask = board::fills::file(occupiedSquares & rankMask);

    return moves & fileMask.inverse();
}

template <Color Side>
constexpr Bitboard pawn_captures(
    const Bitboard startingPawns, const Bitboard enemyPieces) noexcept
{
    return patterns::pawn_attacks<Side>(startingPawns) & enemyPieces;
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

    // occluded fills exclude blockers, but include the sliding piece start squares
    namespace occluded_fills {

        static constexpr auto notAFile = board::masks::files::A.inverse();
        static constexpr auto notHFile = board::masks::files::H.inverse();

        [[nodiscard, gnu::const]] constexpr Bitboard north(
            Bitboard rooks, Bitboard empty) noexcept
        {
            rooks |= empty & (rooks << 8uz);
            empty &= (empty << 8uz);
            rooks |= empty & (rooks << 16uz);
            empty &= (empty << 16uz);
            rooks |= empty & (rooks << 32uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard south(
            Bitboard rooks, Bitboard empty) noexcept
        {
            rooks |= empty & (rooks >> 8uz);
            empty &= (empty >> 8uz);
            rooks |= empty & (rooks >> 16uz);
            empty &= (empty >> 16uz);
            rooks |= empty & (rooks >> 32uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard east(
            Bitboard rooks, Bitboard empty) noexcept
        {
            empty &= notAFile;

            rooks |= empty & (rooks << 1uz);
            empty &= (empty << 1uz);
            rooks |= empty & (rooks << 2uz);
            empty &= (empty << 2uz);
            rooks |= empty & (rooks << 4uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard west(
            Bitboard rooks, Bitboard empty) noexcept
        {
            empty &= notHFile;

            rooks |= empty & (rooks >> 1uz);
            empty &= (empty >> 1uz);
            rooks |= empty & (rooks >> 2uz);
            empty &= (empty >> 2uz);
            rooks |= empty & (rooks >> 4uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard northeast(
            Bitboard bishops, Bitboard empty) noexcept
        {
            empty &= notAFile;

            bishops |= empty & (bishops << 9uz);
            empty &= (empty << 9uz);
            bishops |= empty & (bishops << 18uz);
            empty &= (empty << 18uz);
            bishops |= empty & (bishops << 36uz);

            return bishops;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard southeast(
            Bitboard bishops, Bitboard empty) noexcept
        {
            empty &= notAFile;

            bishops |= empty & (bishops >> 7uz);
            empty &= (empty >> 7uz);
            bishops |= empty & (bishops >> 14uz);
            empty &= (empty >> 14uz);
            bishops |= empty & (bishops >> 28uz);

            return bishops;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard northwest(
            Bitboard bishops, Bitboard empty) noexcept
        {
            empty &= notHFile;

            bishops |= empty & (bishops << 7uz);
            empty &= (empty << 7uz);
            bishops |= empty & (bishops << 14uz);
            empty &= (empty << 14uz);
            bishops |= empty & (bishops << 28uz);

            return bishops;
        }

        [[nodiscard, gnu::const]] constexpr Bitboard southwest(
            Bitboard bishops, Bitboard empty) noexcept
        {
            empty &= notHFile;

            bishops |= empty & (bishops >> 9uz);
            empty &= (empty >> 9uz);
            bishops |= empty & (bishops >> 18uz);
            empty &= (empty >> 18uz);
            bishops |= empty & (bishops >> 36uz);

            return bishops;
        }

    } // namespace occluded_fills

    namespace shifts = board::shifts;

    [[nodiscard, gnu::const]] constexpr Bitboard rook_attacks(
        const Bitboard rooks, const Bitboard emptySquares) noexcept
    {
        const auto northAttacks = shifts::north(occluded_fills::north(rooks, emptySquares));
        const auto southAttacks = shifts::south(occluded_fills::south(rooks, emptySquares));
        const auto eastAttacks  = shifts::east(occluded_fills::east(rooks, emptySquares));
        const auto westAttacks  = shifts::west(occluded_fills::west(rooks, emptySquares));

        return northAttacks | southAttacks | eastAttacks | westAttacks;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard bishop_attacks(
        const Bitboard bishops, const Bitboard emptySquares) noexcept
    {
        const auto NEattacks = shifts::northeast(occluded_fills::northeast(bishops, emptySquares));
        const auto SEattacks = shifts::southeast(occluded_fills::southeast(bishops, emptySquares));
        const auto NWattacks = shifts::northwest(occluded_fills::northwest(bishops, emptySquares));
        const auto SWattacks = shifts::southwest(occluded_fills::southwest(bishops, emptySquares));

        return NEattacks | SEattacks | NWattacks | SWattacks;
    }

} // namespace detail

constexpr Bitboard rook(
    const Bitboard startingRooks, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    return detail::rook_attacks(startingRooks, occupiedSquares.inverse())
         & friendlyPieces.inverse();
}

constexpr Bitboard bishop(
    const Bitboard startingBishops, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    return detail::bishop_attacks(startingBishops, occupiedSquares.inverse())
         & friendlyPieces.inverse();
}

constexpr Bitboard queen(
    const Bitboard startingQueens, const Bitboard occupiedSquares, const Bitboard friendlyPieces) noexcept
{
    const auto emptySquares = occupiedSquares.inverse();

    const auto attacks = detail::rook_attacks(startingQueens, emptySquares)
                       | detail::bishop_attacks(startingQueens, emptySquares);

    return attacks & friendlyPieces.inverse();
}

} // namespace chess::moves::pseudo_legal
