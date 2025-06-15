/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <array>
#include <cmath>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <utility>

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

    // NB. I tried adding a bonus for bishops on open diagonals, but that seemed to make the engine weaker.
    // I've also tried adding a bonus for the bishop pair that increased with fewer pawns on the board, and
    // also a bonus for knights when there are more pawns on the board, but each of those individually and
    // combined seemed to make the engine weaker.

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

    // NB. I tried applying a penalty for isolated pawns, but it made the engine weaker

    template <Color Side>
    [[nodiscard, gnu::const]] int score_side_passed_pawns(
        const Position& position)
    {
        using board::Rank;

        static constexpr auto ROOK_BEHIND_BONUS = 25;
        static constexpr auto KING_ESCORT_BONUS = 2;

        const auto passers = position.get_passed_pawns<Side>();

        const auto& ourPieces = position.pieces_for<Side>();

        auto score { 0 };

        const auto rooks = ourPieces.rooks;
        const auto king  = ourPieces.get_king_location();

        for (const auto pawn : passers.subboards()) {
            const auto mask = board::fills::pawn_rear<Side>(pawn);

            if ((mask & rooks).any())
                score += ROOK_BEHIND_BONUS;

            const auto square = board::Square::from_index(pawn.first());

            // bonus for king closer to passed pawn
            {
                static constexpr auto MAX_DIST = 7uz;

                const auto kingDist = board::chebyshev_distance(king, square);

                score += static_cast<int>(MAX_DIST - kingDist) * KING_ESCORT_BONUS;
            }

            const auto squaresFromPromoting
                = Side == Color::White
                    ? std::to_underlying(Rank::Eight) - std::to_underlying(square.rank)
                    : std::to_underlying(square.rank) - std::to_underlying(Rank::One);

            static constexpr std::array bonuses {
                10000, 100, 85, 70, 60, 50, 35
            };

            score += bonuses.at(squaresFromPromoting);
        }

        return score;
    }

    [[nodiscard, gnu::const]] int score_passed_pawns(
        const Position& position)
    {
        const auto whiteScore = score_side_passed_pawns<Color::White>(position);
        const auto blackScore = score_side_passed_pawns<Color::Black>(position);

        const bool isWhite = position.sideToMove == Color::White;

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return ourScore - theirScore;
    }

    // this "mop up" function gives a bonus for cornering the enemy king in the endgame
    // this can help to prevent draws when you're up material
    [[nodiscard, gnu::const]] int score_endgame_mopup(
        const Position& position, const float endgameWeight, const int materialScore)
    {
        if (std::cmp_greater(materialScore, piece_values::PAWN * 2uz)) {
            const auto ourKing   = position.our_pieces().get_king_location();
            const auto theirKing = position.their_pieces().get_king_location();

            // bonus for forcing enemy king to edge of board
            auto score = static_cast<int>(board::center_manhattan_distance(theirKing)) * 10;

            // use ortho distance to encourage direct opposition
            score += (14 - static_cast<int>(board::manhattan_distance(ourKing, theirKing))) * 4;

            return static_cast<int>(std::round(static_cast<float>(score) * endgameWeight));
        }

        return 0;
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

    const auto endgameWeight = endgame_phase_weight(position);

    const auto materialScore = score_material(position);

    return materialScore
         + score_piece_placement(position)
         + score_rook_files(position)
         + score_connected_rooks(position)
         + score_king_safety(position, endgameWeight)
         + score_squares_controlled_around_kings(position)
         + score_passed_pawns(position)
         + score_endgame_mopup(position, endgameWeight, materialScore);
}

} // namespace chess::eval
