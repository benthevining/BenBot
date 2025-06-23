/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include "PawnStructure.hpp" // NOLINT(build/include_subdir)
#include <cmath>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <utility>

namespace chess::eval {

namespace {

    // Things I've tried that seemed to make the engine weaker:
    // - bonus for bishops on open diagonals
    // - bonus for the bishop pair that increases with fewer pawns on the board, and also a bonus for knights when there are more pawns on the board

    using board::Pieces;
    using pieces::Color;

    namespace masks = board::masks;

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
        static constexpr auto OPEN_FILE_BONUS = 10; // extra bonus for connected rooks on open file

        auto score_connected_rooks = [&position](const Pieces& pieces) {
            auto score { 0 };

            // I originally tried adding a smaller bonus for connected rooks on any rank or file,
            // but experimental results showed that the engine was stronger with just the bonus
            // for connected rooks on open files

            for (const auto file : position.get_open_files()) {
                const auto mask = masks::files::get(file);

                if ((mask & pieces.rooks).count() > 1uz)
                    score += OPEN_FILE_BONUS;
            }

            return score;
        };

        return score_connected_rooks(position.our_pieces()) - score_connected_rooks(position.their_pieces());
    }

    // awards various penalties for king danger
    [[nodiscard, gnu::const]] int score_king_safety(
        const Position& position, const float endgameWeight) noexcept
    {
        static constexpr auto OPEN_KING_PENALTY        = -50;
        static constexpr auto STRANDED_KING_PENALTY    = -75;
        static constexpr auto ATTACKING_KNIGHT_PENALTY = -3;
        static constexpr auto ATTACKING_QUEEN_PENALTY  = -7;

        auto score_side_king = [&position, endgameWeight,
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

            // open/stranded king penalties matter less in endgame
            score = static_cast<int>(std::round(static_cast<float>(score) * (1.f - endgameWeight)));

            // enemy knights & queens near king

            for (const auto knightPos : enemyPieces.knights.squares()) {
                static constexpr auto MAX_DISTANCE = 6uz;

                const auto distance = knight_distance(location, knightPos);

                // penalty increases with smaller distance
                score += static_cast<int>(MAX_DISTANCE - distance) * ATTACKING_KNIGHT_PENALTY;
            }

            for (const auto queenPos : enemyPieces.queens.squares()) {
                static constexpr auto MAX_DISTANCE = 7uz;

                const auto distance = chebyshev_distance(location, queenPos);

                // penalty increases with smaller distance
                score += static_cast<int>(MAX_DISTANCE - distance) * ATTACKING_QUEEN_PENALTY;
            }

            static constexpr auto STARTING_NON_PAWN_MATERIAL
                = piece_values::QUEEN + (piece_values::ROOK * 2) + (piece_values::BISHOP * 2) + (piece_values::KNIGHT * 2);

            // weight score by opponent's remaining material
            return score * (detail::count_material(enemyPieces, false) / STARTING_NON_PAWN_MATERIAL);
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
        // We give a penalty if the opponent attacks more squares around our king than we do.
        // One detail here is that in calculating the attack sets, the defender's king isn't
        // included in determining the number of squares we defend around the king - it's more
        // about the number of pieces/pawns that are defending the king. However, we do count
        // king attacks offensively against squares surrounding the opponent's king.

        const auto surroundingWhiteKing = moves::patterns::king(position.whitePieces.king);

        const auto whiteControlsAroundWK = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, surroundingWhiteKing, position.blackPieces.occupied, false));
        const auto blackControlsAroundWK = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, surroundingWhiteKing, position.whitePieces.occupied));

        const auto whiteScore = whiteControlsAroundWK - blackControlsAroundWK;

        const auto surroundingBlackKing = moves::patterns::king(position.blackPieces.king);

        const auto whiteControlsAroundBK = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, surroundingBlackKing, position.blackPieces.occupied));
        const auto blackControlsAroundBK = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, surroundingBlackKing, position.whitePieces.occupied, false));

        const auto blackScore = blackControlsAroundBK - whiteControlsAroundBK;

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return (ourScore - theirScore) * 2;
    }

    [[nodiscard, gnu::const]] int score_center_control(
        const Position& position) noexcept
    {
        const auto whiteControls = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, masks::CENTER, position.blackPieces.occupied, false));
        const auto blackControls = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, masks::CENTER, position.whitePieces.occupied, false));

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourControl   = isWhite ? whiteControls : blackControls;
        const auto theirControl = isWhite ? blackControls : whiteControls;

        return ourControl - theirControl;
    }

    // this "mop up" function gives a bonus for cornering the enemy king in the endgame
    // this can help to prevent draws when you're up material
    [[nodiscard, gnu::const]] int score_endgame_mopup(
        const Position& position, const float endgameWeight, const int materialScore)
    {
        // only give a mop up score if we're the one up material (and on the attack)
        if (std::cmp_greater(materialScore, piece_values::PAWN * 2uz)) {
            const auto ourKing   = position.our_pieces().get_king_location();
            const auto theirKing = position.their_pieces().get_king_location();

            // bonus for forcing enemy king to edge of board
            auto score = static_cast<int>(center_manhattan_distance(theirKing)) * 10;

            // use ortho distance to encourage direct opposition
            score += (14 - static_cast<int>(manhattan_distance(ourKing, theirKing))) * 4;

            return static_cast<int>(std::round(static_cast<float>(score) * endgameWeight));
        }

        return 0;
    }

    // gives a bonus if we have at least 1 non-pawn piece left and our opponent doesn't
    // in cases such as having a queen & piece vs a rook, this prompts the engine to
    // consider sac'ing the queen for the rook, to eliminate the opponent's last piece
    [[nodiscard, gnu::const]] int no_pieces_left_bonus(
        const Position& position) noexcept
    {
        static constexpr auto LAST_PIECE_BONUS = 500;

        auto num_non_pawn_pieces = [](const Pieces& pieces) {
            return pieces.knights.count() + pieces.bishops.count()
                 + pieces.rooks.count() + pieces.queens.count();
        };

        const auto ourNumPieces   = num_non_pawn_pieces(position.our_pieces());
        const auto theirNumPieces = num_non_pawn_pieces(position.their_pieces());

        if (ourNumPieces == 0uz) {
            // we have no non-pawn pieces left, give a penalty if our opponent does
            if (theirNumPieces > 0uz)
                return -LAST_PIECE_BONUS;

            return 0;
        }

        // we have at least one piece left, award the bonus if our opponent doesn't
        if (theirNumPieces == 0uz)
            return LAST_PIECE_BONUS;

        return 0;
    }

    // a crude way to evaluate "space":
    // we take each side's pawn rearfill, and look at how many more of those squares are controlled by that side than by their opponent
    // this serves to discourage the engine from overextending, but also to incentivize expanding the pawn line to claim more space
    [[nodiscard, gnu::const]] int score_space(
        const Position& position) noexcept
    {
        const auto behindWhitePawns = board::fills::pawn_rear<Color::White>(position.whitePieces.pawns);
        const auto behindBlackPawns = board::fills::pawn_rear<Color::Black>(position.blackPieces.pawns);

        const auto whiteSquares
            = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, behindWhitePawns, position.blackPieces.occupied))
            - static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, behindWhitePawns, position.whitePieces.occupied));

        const auto blackSquares
            = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, behindBlackPawns, position.whitePieces.occupied))
            - static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, behindBlackPawns, position.blackPieces.occupied));

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourSquares   = isWhite ? whiteSquares : blackSquares;
        const auto theirSquares = isWhite ? blackSquares : whiteSquares;

        return (ourSquares - theirSquares) * 2;
    }

} // namespace

int evaluate(const Position& position)
{
    const auto endgameWeight = endgame_phase_weight(position);

    const auto materialScore = score_material(position);

    return materialScore
         + no_pieces_left_bonus(position)
         + score_piece_placement(position)
         + score_rook_files(position)
         + score_connected_rooks(position)
         + score_king_safety(position, endgameWeight)
         + score_squares_controlled_around_kings(position)
         + score_center_control(position)
         + score_endgame_mopup(position, endgameWeight, materialScore)
         + score_space(position)
         + detail::score_pawn_structure(position);
}

} // namespace chess::eval
