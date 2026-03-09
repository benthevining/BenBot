/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for generating pseudo-legal moves.

    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Shifts.hpp>
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
using pieces::Color;

/// @ingroup moves
/// @{

/** Calculates all pseudo-legal pawn pushes. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto pawn_pushes(
    Bitboard startingPawns, Bitboard emptySquares) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal pawn double pushes. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto pawn_double_pushes(
    Bitboard startingPawns, Bitboard occupiedSquares) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal pawn captures.
    The returned bitboard has 1 bits set where each pawn would land after making a capture.
    Note that this function does not generate possible en passant captures.
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto pawn_captures(
    Bitboard startingPawns, Bitboard enemyPieces) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal knight moves. */
[[nodiscard, gnu::const]] constexpr auto knight(
    Bitboard startingKnights, Bitboard friendlyPieces) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal bishop moves.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr auto bishop(
    Bitboard startingBishops, Bitboard emptySquares, Bitboard friendlyPieces) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal rook moves, taking blocking pieces into consideration.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr auto rook(
    Bitboard startingRooks, Bitboard emptySquares, Bitboard friendlyPieces) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal queen moves, taking blocking pieces into consideration.

    The returned move set includes possible captures (i.e., rays ending where an enemy piece
    is located), and also considers blocking friendly pieces.
 */
[[nodiscard, gnu::const]] constexpr auto queen(
    Bitboard startingQueens, Bitboard emptySquares, Bitboard friendlyPieces) noexcept
    -> Bitboard;

/** Calculates all pseudo-legal king moves. */
[[nodiscard, gnu::const]] constexpr auto king(
    Bitboard startingKing, Bitboard friendlyPieces) noexcept
    -> Bitboard;

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
constexpr auto pawn_pushes(
    const Bitboard startingPawns, const Bitboard emptySquares) noexcept
    -> Bitboard
{
    return patterns::pawn_pushes<Side>(startingPawns) & emptySquares;
}

template <Color Side>
constexpr auto pawn_double_pushes(
    const Bitboard startingPawns, const Bitboard occupiedSquares) noexcept
    -> Bitboard
{
    namespace ranks = board::masks::ranks;

    const auto moves = patterns::pawn_double_pushes<Side>(startingPawns) & occupiedSquares.inverse();

    // Need to filter out any pushes that would jump over a piece on the third/sixth rank
    static constexpr auto rankMask = Side == Color::White ? ranks::THREE : ranks::SIX;

    const auto fileMask = board::fills::file(occupiedSquares & rankMask);

    return moves & fileMask.inverse();
}

template <Color Side>
constexpr auto pawn_captures(
    const Bitboard startingPawns, const Bitboard enemyPieces) noexcept
    -> Bitboard
{
    return patterns::pawn_attacks<Side>(startingPawns) & enemyPieces;
}

constexpr auto knight(
    const Bitboard startingKnights, const Bitboard friendlyPieces) noexcept
    -> Bitboard
{
    return patterns::knight(startingKnights) & friendlyPieces.inverse();
}

constexpr auto king(
    const Bitboard startingKing, const Bitboard friendlyPieces) noexcept
    -> Bitboard
{
    return patterns::king(startingKing) & friendlyPieces.inverse();
}

#ifndef DOXYGEN
namespace detail {

    // Sliding piece attack generation is implemented using the Kogge-Stone fill algorithms
    // occluded fills exclude blockers, but include the sliding piece start squares
    namespace occluded_fills {

        namespace files = board::masks::files;

        inline constexpr auto notAFile = files::A.inverse();
        inline constexpr auto notHFile = files::H.inverse();

        [[nodiscard, gnu::const]] constexpr auto north(
            Bitboard rooks, Bitboard empty) noexcept -> Bitboard
        {
            rooks |= empty & (rooks << 8uz);
            empty &= (empty << 8uz);
            rooks |= empty & (rooks << 16uz);
            empty &= (empty << 16uz);
            rooks |= empty & (rooks << 32uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr auto south(
            Bitboard rooks, Bitboard empty) noexcept -> Bitboard
        {
            rooks |= empty & (rooks >> 8uz);
            empty &= (empty >> 8uz);
            rooks |= empty & (rooks >> 16uz);
            empty &= (empty >> 16uz);
            rooks |= empty & (rooks >> 32uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr auto east(
            Bitboard rooks, Bitboard empty) noexcept -> Bitboard
        {
            empty &= notAFile;

            rooks |= empty & (rooks << 1uz);
            empty &= (empty << 1uz);
            rooks |= empty & (rooks << 2uz);
            empty &= (empty << 2uz);
            rooks |= empty & (rooks << 4uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr auto west(
            Bitboard rooks, Bitboard empty) noexcept -> Bitboard
        {
            empty &= notHFile;

            rooks |= empty & (rooks >> 1uz);
            empty &= (empty >> 1uz);
            rooks |= empty & (rooks >> 2uz);
            empty &= (empty >> 2uz);
            rooks |= empty & (rooks >> 4uz);

            return rooks;
        }

        [[nodiscard, gnu::const]] constexpr auto northeast(
            Bitboard bishops, Bitboard empty) noexcept -> Bitboard
        {
            empty &= notAFile;

            bishops |= empty & (bishops << 9uz);
            empty &= (empty << 9uz);
            bishops |= empty & (bishops << 18uz);
            empty &= (empty << 18uz);
            bishops |= empty & (bishops << 36uz);

            return bishops;
        }

        [[nodiscard, gnu::const]] constexpr auto southeast(
            Bitboard bishops, Bitboard empty) noexcept -> Bitboard
        {
            empty &= notAFile;

            bishops |= empty & (bishops >> 7uz);
            empty &= (empty >> 7uz);
            bishops |= empty & (bishops >> 14uz);
            empty &= (empty >> 14uz);
            bishops |= empty & (bishops >> 28uz);

            return bishops;
        }

        [[nodiscard, gnu::const]] constexpr auto northwest(
            Bitboard bishops, Bitboard empty) noexcept -> Bitboard
        {
            empty &= notHFile;

            bishops |= empty & (bishops << 7uz);
            empty &= (empty << 7uz);
            bishops |= empty & (bishops << 14uz);
            empty &= (empty << 14uz);
            bishops |= empty & (bishops << 28uz);

            return bishops;
        }

        [[nodiscard, gnu::const]] constexpr auto southwest(
            Bitboard bishops, Bitboard empty) noexcept -> Bitboard
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

    [[nodiscard, gnu::const]] constexpr auto rook_attacks(
        const Bitboard rooks, const Bitboard emptySquares) noexcept
        -> Bitboard
    {
        const auto northAttacks = shifts::north(occluded_fills::north(rooks, emptySquares));
        const auto southAttacks = shifts::south(occluded_fills::south(rooks, emptySquares));
        const auto eastAttacks  = shifts::east(occluded_fills::east(rooks, emptySquares));
        const auto westAttacks  = shifts::west(occluded_fills::west(rooks, emptySquares));

        return northAttacks | southAttacks | eastAttacks | westAttacks;
    }

    [[nodiscard, gnu::const]] constexpr auto bishop_attacks(
        const Bitboard bishops, const Bitboard emptySquares) noexcept
        -> Bitboard
    {
        const auto NEattacks = shifts::northeast(occluded_fills::northeast(bishops, emptySquares));
        const auto SEattacks = shifts::southeast(occluded_fills::southeast(bishops, emptySquares));
        const auto NWattacks = shifts::northwest(occluded_fills::northwest(bishops, emptySquares));
        const auto SWattacks = shifts::southwest(occluded_fills::southwest(bishops, emptySquares));

        return NEattacks | SEattacks | NWattacks | SWattacks;
    }

} // namespace detail
#endif // DOXYGEN

constexpr auto rook(
    const Bitboard startingRooks, const Bitboard emptySquares, const Bitboard friendlyPieces) noexcept
    -> Bitboard
{
    return detail::rook_attacks(startingRooks, emptySquares) & friendlyPieces.inverse();
}

constexpr auto bishop(
    const Bitboard startingBishops, const Bitboard emptySquares, const Bitboard friendlyPieces) noexcept
    -> Bitboard
{
    return detail::bishop_attacks(startingBishops, emptySquares) & friendlyPieces.inverse();
}

constexpr auto queen(
    const Bitboard startingQueens, const Bitboard emptySquares, const Bitboard friendlyPieces) noexcept
    -> Bitboard
{
    const auto attacks = detail::rook_attacks(startingQueens, emptySquares)
                       | detail::bishop_attacks(startingQueens, emptySquares);

    return attacks & friendlyPieces.inverse();
}

} // namespace chess::moves::pseudo_legal
