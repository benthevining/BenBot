/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/board/Pieces.hpp>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/Colors.hpp>

namespace chess::eval {

namespace {

    using board::Pieces;

    [[nodiscard, gnu::const]] Value material_score(
        const Pieces& ourPieces, const Pieces& theirPieces) noexcept
    {
        return static_cast<Value>(ourPieces.material())
             - static_cast<Value>(theirPieces.material());
    }

    [[nodiscard, gnu::const]] bool is_draw_by_insufficient_material(
        const Position& position) noexcept
    {
        // king vs king+knight or king vs king+bishop

        const auto& whitePieces = position.whitePieces;
        const auto& blackPieces = position.blackPieces;

        // even if either side has a single pawn that can't move, mate can still be possible
        if (whitePieces.pawns.any() || blackPieces.pawns.any()
            || whitePieces.rooks.any() || blackPieces.rooks.any()
            || whitePieces.queens.any() || blackPieces.queens.any()) {
            return false;
        }

        const bool whiteHasOnlyKing = ! (whitePieces.knights.any() || whitePieces.bishops.any());
        const bool blackHasOnlyKing = ! (blackPieces.knights.any() || blackPieces.bishops.any());

        if (whiteHasOnlyKing && blackHasOnlyKing)
            return true;

        if (! (whiteHasOnlyKing || blackHasOnlyKing))
            return false;

        // check if side without the lone king has only 1 knight/bishop
        if (whiteHasOnlyKing)
            return blackPieces.knights.count() + blackPieces.bishops.count() == 1uz;

        return whitePieces.knights.count() + whitePieces.bishops.count() == 1uz;
    }

} // namespace

Value evaluate(const Position& position)
{
    using pieces::Color;

    if (position.is_threefold_repetition() || position.is_fifty_move_draw())
        return DRAW;

    if (is_draw_by_insufficient_material(position))
        return DRAW;

    if (! moves::any_legal_moves(position)) {
        if (position.is_check())
            return MIN; // we got mated

        return DRAW; // stalemate
    }

    const auto& ourPieces   = position.sideToMove == Color::White ? position.whitePieces : position.blackPieces;
    const auto& theirPieces = position.sideToMove == Color::White ? position.blackPieces : position.whitePieces;

    return material_score(ourPieces, theirPieces);

    // TODO: add piece square table values
}

} // namespace chess::eval
