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

// Things I've tried that seemed to make the engine weaker:
// - bonus for bishops on open diagonals
// - bonus for the bishop pair that increases with fewer pawns on the board, and also a bonus for knights when there are more pawns on the board

#include "eval/PawnStructure.hpp"
#include "eval/Positional.hpp"
#include <algorithm>
#include <cmath> // IWYU pragma: keep - for std::round
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <utility>

namespace ben_bot::eval {

namespace {

    // returns a [0..1] value that is 0 at the start of the game and 1 in the late endgame
    [[nodiscard, gnu::pure]] constexpr auto endgame_phase_weight(
        const Position& position) noexcept
        -> float
    {
        // game phase is roughly determined based on the total amount of non-pawn material left on the board
        // we say that the endgame has begun once the queens & two pairs of minor pieces have been traded
        static constexpr auto EG_MATERIAL_START_ONE_SIDE = static_cast<float>(
            (piece_values::ROOK * 2uz) + piece_values::BISHOP + piece_values::KNIGHT);

        static constexpr auto EG_MATERIAL_START = EG_MATERIAL_START_ONE_SIDE * 2.f;

        static constexpr auto multiplier = 1.f / EG_MATERIAL_START;

        const auto nonPawnMaterialLeft = detail::count_material<false>(position.whitePieces)
                                       + detail::count_material<false>(position.blackPieces);

        const auto pcntLeft = static_cast<float>(nonPawnMaterialLeft) * multiplier;

        return 1.f - std::min(1.f, pcntLeft);
    }

    using chess::board::Bitboard;
    using chess::board::Pieces;
    using chess::pieces::Color;

    namespace masks = chess::board::masks;

    // awards a bonus for rooks on open or half-open files
    [[nodiscard, gnu::pure]] auto score_rook_files(
        const Position& position) noexcept
        -> int
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
    [[nodiscard, gnu::pure]] auto score_connected_rooks(
        const Position& position) noexcept
        -> int
    {
        static constexpr auto OPEN_FILE_BONUS = 10; // extra bonus for connected rooks on open file

        auto score_connected_rooks = [&position](const Pieces& pieces) {
            auto score { 0 };

            // I originally tried adding a smaller bonus for connected rooks on any rank or file,
            // but experimental results showed that the engine was stronger with just the bonus
            // for connected rooks on open files

            for (const auto file : position.get_open_files()) {
                if (const auto mask = masks::files::get(file);
                    (mask & pieces.rooks).count() > 1uz) {
                    score += OPEN_FILE_BONUS;
                }
            }

            return score;
        };

        return score_connected_rooks(position.our_pieces()) - score_connected_rooks(position.their_pieces());
    }

    // awards various penalties for king danger
    [[nodiscard, gnu::pure]] auto score_king_safety(
        const Position& position, const float endgameWeight) noexcept
        -> int
    {
        static constexpr auto OPEN_KING_PENALTY        = -50;
        static constexpr auto STRANDED_KING_PENALTY    = -75;
        static constexpr auto ATTACKING_KNIGHT_PENALTY = -3;
        static constexpr auto ATTACKING_QUEEN_PENALTY  = -7;

        auto score_side_king = [&position, endgameWeight,
                                   allPawns = position.whitePieces.pawns | position.blackPieces.pawns](
                                   const Pieces& pieces, const chess::game::CastlingRights& castlingRights,
                                   const Pieces& enemyPieces) {
            auto score { 0 };

            const auto location = pieces.get_king_location();

            // king on open file or diagonal
            if (position.is_file_half_open(location.file))
                score += (OPEN_KING_PENALTY / 2);
            else if (position.is_file_open(location.file)
                     or (masks::diagonal(location) & allPawns).none()
                     or (masks::antidiagonal(location) & allPawns).none())
                score += OPEN_KING_PENALTY;

            using chess::board::File;

            // king stranded in center without castling rights
            if (castlingRights.neither()
                and (location.file == File::D or location.file == File::E))
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
            return score * (detail::count_material<false>(enemyPieces) / STARTING_NON_PAWN_MATERIAL);
        };

        const bool isWhite = position.is_white_to_move();

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

    // Attack sets for one side, calculated once per evaluate() call & shared between
    // the mobility & king zone terms
    struct SideAttacks final {
        // all squares attacked by pawns, knights, bishops, rooks & queens (not the king)
        Bitboard attacked;

        // total mobility bonus for this side's pieces, before blending by game phase
        int mobilityMG { 0 };
        int mobilityEG { 0 };
    };

    // Mobility weights. Each piece gets (number of moves - baseline) * weight, so that an
    // average piece scores about 0. We count pseudo-legal moves, not counting squares
    // occupied by our own pieces or attacked by enemy pawns.
    namespace mobility {

        struct Weights final {
            int baseline;
            int middlegame;
            int endgame;
        };

        inline constexpr Weights KNIGHT { .baseline = 4, .middlegame = 4, .endgame = 4 };
        inline constexpr Weights BISHOP { .baseline = 6, .middlegame = 5, .endgame = 5 };
        inline constexpr Weights ROOK { .baseline = 7, .middlegame = 2, .endgame = 4 };
        inline constexpr Weights QUEEN { .baseline = 13, .middlegame = 1, .endgame = 2 };

    } // namespace mobility

    template <Color Side>
    [[nodiscard, gnu::pure]] auto calculate_attacks(
        const Position& position, const Bitboard ourPawnAttacks, const Bitboard enemyPawnAttacks) noexcept
        -> SideAttacks
    {
        namespace magics   = chess::moves::magics;
        namespace patterns = chess::moves::patterns;

        const auto& pieces = position.pieces_for<Side>();

        const auto friendly = pieces.occupied;
        const auto occupied = position.occupied();

        const auto mobilityArea = (friendly | enemyPawnAttacks).inverse();

        // pawn & knight attacks include squares occupied by friendly pieces, but slider attacks
        // don't (same as chess::moves::num_squares_attacked())
        SideAttacks result {
            .attacked = ourPawnAttacks | patterns::knight(pieces.knights)
        };

        auto add_mobility = [&result, mobilityArea](
                                const Bitboard moves, const mobility::Weights& weights) {
            const auto numMoves = static_cast<int>((moves & mobilityArea).count()) - weights.baseline;

            result.mobilityMG += numMoves * weights.middlegame;
            result.mobilityEG += numMoves * weights.endgame;
        };

        for (const auto square : pieces.knights.squares())
            add_mobility(patterns::knight(Bitboard::from_square(square)), mobility::KNIGHT);

        for (const auto square : pieces.bishops.squares()) {
            const auto moves = magics::bishop(square, occupied, friendly);
            result.attacked |= moves;
            add_mobility(moves, mobility::BISHOP);
        }

        for (const auto square : pieces.rooks.squares()) {
            const auto moves = magics::rook(square, occupied, friendly);
            result.attacked |= moves;
            add_mobility(moves, mobility::ROOK);
        }

        for (const auto square : pieces.queens.squares()) {
            const auto moves = magics::queen(square, occupied, friendly);
            result.attacked |= moves;
            add_mobility(moves, mobility::QUEEN);
        }

        return result;
    }

    // bonus for pieces with more available moves
    [[nodiscard, gnu::pure]] auto score_mobility(
        const SideAttacks& ours, const SideAttacks& theirs, const float endgameWeight) noexcept
        -> int
    {
        const auto middlegame = static_cast<float>(ours.mobilityMG - theirs.mobilityMG);
        const auto endgame    = static_cast<float>(ours.mobilityEG - theirs.mobilityEG);

        return static_cast<int>(std::round((middlegame * (1.f - endgameWeight)) + (endgame * endgameWeight)));
    }

    [[nodiscard, gnu::pure]] auto score_squares_controlled_around_kings(
        const Position& position, const SideAttacks& white, const SideAttacks& black) noexcept
        -> int
    {
        namespace patterns = chess::moves::patterns;

        // We give a penalty if the opponent attacks more squares around our king than we do.
        // One detail here is that in calculating the attack sets, the defender's king isn't
        // included in determining the number of squares we defend around the king - it's more
        // about the number of pieces/pawns that are defending the king. However, we do count
        // king attacks offensively against squares surrounding the opponent's king.

        const auto whiteKingAttacks = patterns::king(position.whitePieces.king);
        const auto blackKingAttacks = patterns::king(position.blackPieces.king);

        const auto whiteAttacksWithKing = white.attacked | whiteKingAttacks;
        const auto blackAttacksWithKing = black.attacked | blackKingAttacks;

        const auto whiteControlsAroundWK = static_cast<int>((white.attacked & whiteKingAttacks).count());
        const auto blackControlsAroundWK = static_cast<int>((blackAttacksWithKing & whiteKingAttacks).count());

        const auto whiteScore = whiteControlsAroundWK - blackControlsAroundWK;

        const auto whiteControlsAroundBK = static_cast<int>((whiteAttacksWithKing & blackKingAttacks).count());
        const auto blackControlsAroundBK = static_cast<int>((black.attacked & blackKingAttacks).count());

        const auto blackScore = blackControlsAroundBK - whiteControlsAroundBK;

        const bool isWhite = position.is_white_to_move();

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return (ourScore - theirScore) * 2;
    }

    // this "mop up" function gives a bonus for cornering the enemy king in the endgame
    // this can help to prevent draws when you're up material
    [[nodiscard, gnu::pure]] auto score_endgame_mopup(
        const Position& position, const float endgameWeight, const int materialScore)
        -> int
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
    [[nodiscard, gnu::pure]] auto no_pieces_left_bonus(
        const Position& position) noexcept
        -> int
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

} // namespace

auto evaluate(
    const Position&          position,
    const PieceSquareTables& pst)
    -> Score
{
    const auto endgameWeight = endgame_phase_weight(position);

    const auto materialScore = score_material(position);

    namespace patterns = chess::moves::patterns;

    const auto whitePawnAttacks = patterns::pawn_attacks<Color::White>(position.whitePieces.pawns);
    const auto blackPawnAttacks = patterns::pawn_attacks<Color::Black>(position.blackPieces.pawns);

    const auto whiteAttacks = calculate_attacks<Color::White>(position, whitePawnAttacks, blackPawnAttacks);
    const auto blackAttacks = calculate_attacks<Color::Black>(position, blackPawnAttacks, whitePawnAttacks);

    const bool isWhite = position.is_white_to_move();

    const auto& ourAttacks   = isWhite ? whiteAttacks : blackAttacks;
    const auto& theirAttacks = isWhite ? blackAttacks : whiteAttacks;

    return Score { static_cast<Value>(
        materialScore
        + no_pieces_left_bonus(position)
        + pst.score_piece_placement(position, endgameWeight)
        + score_rook_files(position)
        + score_connected_rooks(position)
        + score_king_safety(position, endgameWeight)
        + score_squares_controlled_around_kings(position, whiteAttacks, blackAttacks)
        + score_mobility(ourAttacks, theirAttacks, endgameWeight)
        + score_endgame_mopup(position, endgameWeight, materialScore)
        + detail::score_positional(position)
        + detail::score_pawn_structure(position)) };
}

} // namespace ben_bot::eval
