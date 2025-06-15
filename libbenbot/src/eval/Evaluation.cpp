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
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

namespace chess::eval {

namespace {

    using board::Pieces;
    using pieces::Color;

    namespace masks = board::masks;

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

        auto score_side_rooks = [&position](const Pieces& pieces) {
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

    // bonus for connected rooks
    [[nodiscard, gnu::const]] int score_connected_rooks(
        const Position& position) noexcept
    {
        static constexpr auto CONNECTED_ROOKS_BONUS = 10;

        auto score_connected_rooks = [emptySquares = position.free()](const Pieces& pieces) {
            auto score { 0 };

            for (const auto rookPos : pieces.rooks.subboards()) {
                // check this way because the top-level rook move generation function prunes squares occupied by friendly pieces,
                // but we want to check if the rook can attack a square occupied by another friendly rook
                const auto attackedSquares = moves::pseudo_legal::detail::rook_attacks(rookPos, emptySquares);

                if ((attackedSquares & pieces.rooks).count() > 1uz)
                    score += CONNECTED_ROOKS_BONUS;
            }

            return score;
        };

        return score_connected_rooks(position.our_pieces()) - score_connected_rooks(position.their_pieces());
    }

    // NB. I tried adding a bonus for bishops on open diagonals, but that seemed to make the engine weaker

    // awards various penalties for king danger
    [[nodiscard, gnu::const]] int score_king_safety(
        const Position& position) noexcept
    {
        static constexpr auto OPEN_KING_PENALTY        = -35;
        static constexpr auto STRANDED_KING_PENALTY    = -75;
        static constexpr auto ATTACKING_KNIGHT_PENALTY = -3;
        static constexpr auto ATTACKING_QUEEN_PENALTY  = -7;

        auto score_side_king = [&position,
                                   allPawns = position.whitePieces.pawns | position.blackPieces.pawns](
                                   const Pieces& pieces, const game::CastlingRights& castlingRights,
                                   const Pieces& enemyPieces) {
            auto score { 0 };

            const auto location = pieces.get_king_location();

            // king on open file or diagonal
            if (position.is_file_half_open(location.file))
                score += (OPEN_KING_PENALTY / 2);
            else if (position.is_file_open(location.file)
                     || (masks::diagonal(location) & allPawns).none()
                     || (masks::antidiagonal(location) & allPawns).none())
                score += OPEN_KING_PENALTY;

            using board::File;

            // king stranded in center without castling rights
            if (castlingRights.neither()
                && (location.file == File::D || location.file == File::E))
                score += STRANDED_KING_PENALTY;

            // enemy knights & queens near king

            for (const auto knightPos : enemyPieces.knights.squares()) {
                static constexpr auto MAX_DISTANCE = 6uz;

                const auto distance = board::knight_distance(location, knightPos);

                // penalty increases with smaller distance
                score += static_cast<int>(MAX_DISTANCE - distance) * ATTACKING_KNIGHT_PENALTY;
            }

            for (const auto queenPos : enemyPieces.queens.squares()) {
                static constexpr auto MAX_DISTANCE = 7uz;

                const auto distance = board::chebyshev_distance(location, queenPos);

                // penalty increases with smaller distance
                score += static_cast<int>(MAX_DISTANCE - distance) * ATTACKING_QUEEN_PENALTY;
            }

            return score;
        };

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourScore = score_side_king(
            position.our_pieces(),
            isWhite ? position.whiteCastlingRights : position.blackCastlingRights,
            position.their_pieces());

        const auto theirScore = score_side_king(
            position.their_pieces(),
            isWhite ? position.blackCastlingRights : position.whiteCastlingRights,
            position.our_pieces());

        return ourScore - theirScore;
    }

    [[nodiscard, gnu::const]] int score_squares_controlled_around_kings(
        const Position& position) noexcept
    {
        const auto surroundingWhiteKing = moves::patterns::king(position.whitePieces.king);

        const auto whiteControlsAroundWK = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, surroundingWhiteKing, position.blackPieces.occupied));
        const auto blackControlsAroundWK = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, surroundingWhiteKing, position.whitePieces.occupied));

        const auto whiteScore = whiteControlsAroundWK - blackControlsAroundWK;

        const auto surroundingBlackKing = moves::patterns::king(position.blackPieces.king);

        const auto whiteControlsAroundBK = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, surroundingBlackKing, position.blackPieces.occupied));
        const auto blackControlsAroundBK = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, surroundingBlackKing, position.whitePieces.occupied));

        const auto blackScore = blackControlsAroundBK - whiteControlsAroundBK;

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return ourScore - theirScore;
    }

} // namespace

int evaluate(const Position& position)
{
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
         + score_rook_files(position)
         + score_connected_rooks(position)
         + score_king_safety(position)
         + score_squares_controlled_around_kings(position);
}

} // namespace chess::eval
