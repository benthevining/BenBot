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
            [[likely]];
            return false;
        }

        const auto numWhiteKnights = whitePieces.knights.count();
        const auto numWhiteBishops = whitePieces.bishops.count();

        const auto numBlackKnights = blackPieces.knights.count();
        const auto numBlackBishops = blackPieces.bishops.count();

        const bool whiteHasOnlyKing = numWhiteKnights + numWhiteBishops == 0uz;
        const bool blackHasOnlyKing = numBlackKnights + numBlackBishops == 0uz;

        if (! (whiteHasOnlyKing || blackHasOnlyKing)) {
            [[likely]];
            return false;
        }

        if (whiteHasOnlyKing && blackHasOnlyKing)
            return true;

        // check if side without the lone king has only 1 knight/bishop

        if (whiteHasOnlyKing)
            return numBlackKnights + numBlackBishops == 1uz;

        return numWhiteKnights + numWhiteBishops == 1uz;
    }

    [[nodiscard, gnu::const]] Value material_score(
        const Pieces& ourPieces, const Pieces& theirPieces) noexcept
    {
        return static_cast<Value>(ourPieces.material())
             - static_cast<Value>(theirPieces.material());
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
