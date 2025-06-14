/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/Colors.hpp>

namespace chess::eval {

namespace {

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

    // awards a bonus for rooks on open or half-open files
    [[nodiscard, gnu::const]] int score_rook_files(
        const Position& position) noexcept
    {
        static constexpr auto HALF_OPEN_FILE_BONUS = 30;
        static constexpr auto OPEN_FILE_BONUS      = 70;

        auto score_side_rooks = [&position](const board::Pieces& pieces) {
            auto score { 0 };

            for (const auto square : pieces.rooks.squares()) {
                if (position.is_file_half_open(square.file))
                    score += HALF_OPEN_FILE_BONUS;
                else if (position.is_file_open(square.file))
                    score += OPEN_FILE_BONUS;
            }

            return score;
        };

        return score_side_rooks(position.our_pieces()) - score_side_rooks(position.their_pieces());
    }

} // namespace

int evaluate(const Position& position)
{
    using pieces::Color;

    if (position.is_threefold_repetition() || position.is_fifty_move_draw())
        return DRAW;

    if (is_draw_by_insufficient_material(position))
        return DRAW;

    if (! moves::any_legal_moves(position)) {
        if (position.is_check())
            return -MATE; // we got mated

        return DRAW; // stalemate
    }

    return score_material(position)
         + score_piece_placement(position)
         + score_rook_files(position);
}

} // namespace chess::eval
