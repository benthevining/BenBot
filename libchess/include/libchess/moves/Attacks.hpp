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
    This file contains functions for calculating various attack sets.
    @ingroup moves
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

namespace chess::moves {

using board::Bitboard;
using board::Pieces;
using pieces::Color;
using std::size_t;

/** Returns true if any of the ``pieces`` attacks any of the ``targetSquares``.
    This function considers only pseudo-legal moves, not strictly legal moves.

    @tparam Side The color that the ``pieces`` represent.

    @ingroup moves
    @see num_squares_attacked()
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto squares_attacked(
    const Pieces& pieces, Bitboard targetSquares, Bitboard enemyPieces) noexcept
    -> bool;

/** Returns the number of ``targetSquares`` that any of the ``pieces`` attack.
    This function considers only pseudo-legal moves, not strictly legal moves.
    King attacks will only be considered if ``includeKing`` is true.

    @tparam Side The color that the ``pieces`` represent.

    @ingroup moves
    @see squares_attacked()
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto num_squares_attacked(
    const Pieces& pieces, Bitboard targetSquares, Bitboard enemyPieces, bool includeKing = true) noexcept
    -> size_t;

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
constexpr auto squares_attacked(
    const Pieces& pieces, const Bitboard targetSquares, const Bitboard enemyPieces) noexcept
    -> bool
{
    // For pawns, knights & kings we use the move pattern generator functions, because the only
    // difference with the pseudo-legal generator functions is excluding squares occupied by
    // friendly pieces, a consideration that is irrelevant here. For sliding pieces, the pseudo-legal
    // generator functions are necessary to consider blocking pieces, but we can use them in a
    // set-wise manner, e.g., to determine if any Queen attacks any of the target squares.

    if (const auto pawnAttacks = patterns::pawn_attacks<Side>(pieces.pawns);
        (pawnAttacks & targetSquares).any()) {
        return true;
    }

    if (const auto knightAttacks = patterns::knight(pieces.knights);
        (knightAttacks & targetSquares).any()) {
        return true;
    }

    const auto friendlyPieces = pieces.occupied;
    const auto emptySquares   = (friendlyPieces | enemyPieces).inverse();

    // TODO: could try replacing the pseudo-legal funcs with the magics funcs here

    if (const auto queenAttacks = pseudo_legal::queen(pieces.queens, emptySquares, friendlyPieces);
        (queenAttacks & targetSquares).any()) {
        return true;
    }

    if (const auto rookAttacks = pseudo_legal::rook(pieces.rooks, emptySquares, friendlyPieces);
        (rookAttacks & targetSquares).any()) {
        return true;
    }

    if (const auto bishopAttacks = pseudo_legal::bishop(pieces.bishops, emptySquares, friendlyPieces);
        (bishopAttacks & targetSquares).any()) {
        return true;
    }

    // test king last
    // this function is used for things like detecting if a position is check,
    // so it's unlikely that the king would be the only relevant attacker of a square

    const auto kingAttacks = patterns::king(pieces.king);

    return (kingAttacks & targetSquares).any();
}

template <Color Side>
constexpr auto num_squares_attacked(
    const Pieces& pieces, const Bitboard targetSquares, const Bitboard enemyPieces, const bool includeKing) noexcept
    -> size_t
{
    const auto friendlyPieces = pieces.occupied;
    const auto emptySquares   = (friendlyPieces | enemyPieces).inverse();

    const auto pawnAttacks   = patterns::pawn_attacks<Side>(pieces.pawns);
    const auto knightAttacks = patterns::knight(pieces.knights);
    const auto queenAttacks  = pseudo_legal::queen(pieces.queens, emptySquares, friendlyPieces);
    const auto rookAttacks   = pseudo_legal::rook(pieces.rooks, emptySquares, friendlyPieces);
    const auto bishopAttacks = pseudo_legal::bishop(pieces.bishops, emptySquares, friendlyPieces);

    auto allAttacks = pawnAttacks | knightAttacks | queenAttacks | rookAttacks | bishopAttacks;

    if (includeKing) {
        allAttacks |= patterns::king(pieces.king);
    }

    return (targetSquares & allAttacks).count();
}

} // namespace chess::moves
