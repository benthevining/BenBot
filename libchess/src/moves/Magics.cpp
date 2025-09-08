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

#include "MagicMoves.hpp" // NOLINT(build/include_subdir)
#include "MagicsImpl.hpp" // NOLINT(build/include_subdir)
#include <libchess/board/Square.hpp>
#include <libchess/moves/Magics.hpp>

namespace chess::moves::magics {

Bitboard bishop(
    const Square& bishopPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
{
    const auto moves = MAGIC_MOVES.at(
        calc_bishop_index(bishopPos.index(), occupiedSquares));

    return moves & friendlyPieces.inverse();
}

Bitboard rook(
    const Square& rookPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
{
    const auto moves = MAGIC_MOVES.at(
        calc_rook_index(rookPos.index(), occupiedSquares));

    return moves & friendlyPieces.inverse();
}

Bitboard queen(
    const Square& queenPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
{
    const auto squareIdx = queenPos.index();

    const auto bishopMoves = MAGIC_MOVES.at(
        calc_bishop_index(squareIdx, occupiedSquares));

    const auto rookMoves = MAGIC_MOVES.at(
        calc_rook_index(squareIdx, occupiedSquares));

    return (bishopMoves | rookMoves) & friendlyPieces.inverse();
}

} // namespace chess::moves::magics
