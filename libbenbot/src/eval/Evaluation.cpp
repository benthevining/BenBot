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
#include "eval/Threats.hpp"
#include <algorithm>
#include <cassert>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/AttackInfo.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <utility>

namespace ben_bot::eval {

namespace {

    // game phase is roughly determined based on the total amount of non-pawn material left on the board
    // we say that the endgame has begun once the queens & two pairs of minor pieces have been traded
    // (the phase uses the fixed piece_values, not the eval's material weights)
    inline constexpr int MAX_PHASE
        = ((piece_values::ROOK * 2) + piece_values::BISHOP + piece_values::KNIGHT) * 2;

    // returns a [0..MAX_PHASE] value that is MAX_PHASE until the endgame starts, then falls
    // linearly to 0 as the remaining non-pawn material is traded off
    [[nodiscard, gnu::pure]] constexpr auto middlegame_phase(
        const Position& position) noexcept
        -> int
    {
        const auto nonPawnMaterialLeft = detail::count_material<false>(position.whitePieces)
                                       + detail::count_material<false>(position.blackPieces);

        return std::min(nonPawnMaterialLeft, MAX_PHASE);
    }

    using chess::board::Bitboard;
    using chess::board::Pieces;
    using chess::pieces::Color;

    namespace masks = chess::board::masks;

    // the material & piece square table terms for one side, from that side's point of view
    template <bool IsBlack>
    [[nodiscard, gnu::pure]] auto score_side_pieces(
        const Pieces& pieces) noexcept
        -> TaperedScore
    {
        TaperedScore score;

        for (const auto type : { PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
                 PieceType::Rook, PieceType::Queen, PieceType::King }) {
            auto board = pieces.get_type(type);

            if (type != PieceType::King)
                score += DEFAULT_WEIGHTS.material(type) * static_cast<int>(board.count());

            // the tables are from White's point of view
            if constexpr (IsBlack) {
                board = chess::board::flips::vertical(board);
            }

            const auto pst = DEFAULT_WEIGHTS.pst(type);

            for (const auto idx : board.indices()) {
                assert(idx < pst.size());
                score += pst[idx];
            }
        }

        return score;
    }

    // material & piece placement
    [[nodiscard, gnu::pure]] auto score_pieces(
        const Position& position) noexcept
        -> TaperedScore
    {
        if (position.is_black_to_move()) {
            return score_side_pieces<true>(position.our_pieces())
                 - score_side_pieces<false>(position.their_pieces());
        }

        return score_side_pieces<false>(position.our_pieces())
             - score_side_pieces<true>(position.their_pieces());
    }

    // awards a bonus for rooks on open or half-open files
    [[nodiscard, gnu::pure]] auto score_rook_files(
        const Position& position) noexcept
        -> TaperedScore
    {
        auto score_side_rooks = [&position](const Pieces& pieces) {
            TaperedScore score;

            for (const auto square : pieces.rooks.squares()) {
                if (position.is_file_half_open(square.file))
                    score += DEFAULT_WEIGHTS.get(Term::RookHalfOpenFile);
                else if (position.is_file_open(square.file))
                    score += DEFAULT_WEIGHTS.get(Term::RookOpenFile);
            }

            return score;
        };

        return score_side_rooks(position.our_pieces()) - score_side_rooks(position.their_pieces());
    }

    // bonus for connected rooks
    [[nodiscard, gnu::pure]] auto score_connected_rooks(
        const Position& position) noexcept
        -> TaperedScore
    {
        auto score_connected_rooks = [&position](const Pieces& pieces) {
            TaperedScore score;

            // I originally tried adding a smaller bonus for connected rooks on any rank or file,
            // but experimental results showed that the engine was stronger with just the bonus
            // for connected rooks on open files

            for (const auto file : position.get_open_files()) {
                if (const auto mask = masks::files::get(file);
                    (mask & pieces.rooks).count() > 1uz) {
                    score += DEFAULT_WEIGHTS.get(Term::ConnectedRooksOpenFile);
                }
            }

            return score;
        };

        return score_connected_rooks(position.our_pieces()) - score_connected_rooks(position.their_pieces());
    }

    // awards various penalties for king danger
    [[nodiscard, gnu::pure]] auto score_king_safety(
        const Position& position) noexcept
        -> TaperedScore
    {
        auto score_side_king = [&position,
                                   allPawns = position.whitePieces.pawns | position.blackPieces.pawns](
                                   const Pieces& pieces, const chess::game::CastlingRights& castlingRights,
                                   const Pieces& enemyPieces) {
            TaperedScore score;

            const auto location = pieces.get_king_location();

            // king on open file or diagonal
            if (position.is_file_half_open(location.file))
                score += DEFAULT_WEIGHTS.get(Term::KingHalfOpenFile);
            else if (position.is_file_open(location.file)
                     or (masks::diagonal(location) & allPawns).none()
                     or (masks::antidiagonal(location) & allPawns).none())
                score += DEFAULT_WEIGHTS.get(Term::KingOpenLine);

            using chess::board::File;

            // king stranded in center without castling rights
            if (castlingRights.neither()
                and (location.file == File::D or location.file == File::E))
                score += DEFAULT_WEIGHTS.get(Term::KingStranded);

            // enemy knights & queens near king

            for (const auto knightPos : enemyPieces.knights.squares()) {
                static constexpr auto MAX_DISTANCE = 6uz;

                const auto distance = knight_distance(location, knightPos);

                // penalty increases with smaller distance
                score += DEFAULT_WEIGHTS.get(Term::KnightTropism) * static_cast<int>(MAX_DISTANCE - distance);
            }

            for (const auto queenPos : enemyPieces.queens.squares()) {
                static constexpr auto MAX_DISTANCE = 7uz;

                const auto distance = chebyshev_distance(location, queenPos);

                // penalty increases with smaller distance
                score += DEFAULT_WEIGHTS.get(Term::QueenTropism) * static_cast<int>(MAX_DISTANCE - distance);
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

    using chess::moves::AttackInfo;

    // Each piece's mobility bonus is looked up by its number of moves. We count pseudo-legal
    // moves, not counting squares occupied by our own pieces or attacked by enemy pawns.
    namespace mobility {

        [[nodiscard, gnu::const]] constexpr auto term_for(const PieceType type) noexcept -> Term
        {
            if (type == PieceType::Knight)
                return Term::KnightMobility;

            if (type == PieceType::Bishop)
                return Term::BishopMobility;

            if (type == PieceType::Rook)
                return Term::RookMobility;

            return Term::QueenMobility;
        }

        // Adds up each side's mobility bonus. compute_attacks() calls this once for each
        // knight, bishop, rook & queen, with that piece's attack set.
        struct Counter final {
            explicit Counter(const Position& position) noexcept
                : whitePieces { position.whitePieces.occupied }
                , blackPieces { position.blackPieces.occupied }
            {
            }

            Bitboard whitePieces;
            Bitboard blackPieces;

            TaperedScore white;
            TaperedScore black;

            void operator()(
                const Color side, const PieceType type, const Bitboard attacks, const AttackInfo& info) noexcept
            {
                const bool isWhite = side == Color::White;

                const auto friendly         = isWhite ? whitePieces : blackPieces;
                const auto enemyPawnAttacks = isWhite ? info.black.pawns : info.white.pawns;

                const auto mobilityArea = (friendly | enemyPawnAttacks).inverse();

                const auto numMoves = (attacks & mobilityArea).count();

                (isWhite ? white : black) += DEFAULT_WEIGHTS.get(term_for(type), numMoves);
            }
        };

    } // namespace mobility

    // bonus for pieces with more available moves
    [[nodiscard, gnu::pure]] auto score_mobility(
        const Position& position, const mobility::Counter& counter) noexcept
        -> TaperedScore
    {
        if (position.is_white_to_move())
            return counter.white - counter.black;

        return counter.black - counter.white;
    }

    [[nodiscard, gnu::pure]] auto score_squares_controlled_around_kings(
        const Position& position, const AttackInfo& attacks, const detail::SquareControl& control) noexcept
        -> TaperedScore
    {
        // We give a penalty if the opponent attacks more squares around our king than we do.
        // One detail here is that in calculating the attack sets, the defender's king isn't
        // included in determining the number of squares we defend around the king - it's more
        // about the number of pieces/pawns that are defending the king. However, we do count
        // king attacks offensively against squares surrounding the opponent's king.

        const auto whiteKingAttacks = attacks.white.king;
        const auto blackKingAttacks = attacks.black.king;

        const auto whiteControlsAroundWK = static_cast<int>((control.white & whiteKingAttacks).count());
        const auto blackControlsAroundWK = static_cast<int>((control.blackWithKing & whiteKingAttacks).count());

        const auto whiteScore = whiteControlsAroundWK - blackControlsAroundWK;

        const auto whiteControlsAroundBK = static_cast<int>((control.whiteWithKing & blackKingAttacks).count());
        const auto blackControlsAroundBK = static_cast<int>((control.black & blackKingAttacks).count());

        const auto blackScore = blackControlsAroundBK - whiteControlsAroundBK;

        const bool isWhite = position.is_white_to_move();

        const auto ourScore   = isWhite ? whiteScore : blackScore;
        const auto theirScore = isWhite ? blackScore : whiteScore;

        return DEFAULT_WEIGHTS.get(Term::KingAreaControl) * (ourScore - theirScore);
    }

    // this "mop up" function gives a bonus for cornering the enemy king in the endgame
    // this can help to prevent draws when you're up material
    [[nodiscard, gnu::pure]] auto score_endgame_mopup(
        const Position& position)
        -> TaperedScore
    {
        // only give a mop up score if we're the one up material (and on the attack)
        // (this uses the fixed piece_values, not the eval's material weights)
        if (std::cmp_greater(score_material(position), piece_values::PAWN * 2uz)) {
            const auto ourKing   = position.our_pieces().get_king_location();
            const auto theirKing = position.their_pieces().get_king_location();

            // bonus for forcing enemy king to edge of board
            const auto edge = DEFAULT_WEIGHTS.get(Term::MopUpEdge) * static_cast<int>(center_manhattan_distance(theirKing));

            // use ortho distance to encourage direct opposition
            const auto distance = DEFAULT_WEIGHTS.get(Term::MopUpKingDistance) * (14 - static_cast<int>(manhattan_distance(ourKing, theirKing)));

            return edge + distance;
        }

        return { };
    }

    // gives a bonus if we have at least 1 non-pawn piece left and our opponent doesn't
    // in cases such as having a queen & piece vs a rook, this prompts the engine to
    // consider sac'ing the queen for the rook, to eliminate the opponent's last piece
    [[nodiscard, gnu::pure]] auto no_pieces_left_bonus(
        const Position& position) noexcept
        -> TaperedScore
    {
        auto num_non_pawn_pieces = [](const Pieces& pieces) {
            return pieces.knights.count() + pieces.bishops.count()
                 + pieces.rooks.count() + pieces.queens.count();
        };

        const auto ourNumPieces   = num_non_pawn_pieces(position.our_pieces());
        const auto theirNumPieces = num_non_pawn_pieces(position.their_pieces());

        if (ourNumPieces == 0uz) {
            // we have no non-pawn pieces left, give a penalty if our opponent does
            if (theirNumPieces > 0uz)
                return -DEFAULT_WEIGHTS.get(Term::LastPiece);

            return { };
        }

        // we have at least one piece left, award the bonus if our opponent doesn't
        if (theirNumPieces == 0uz)
            return DEFAULT_WEIGHTS.get(Term::LastPiece);

        return { };
    }

} // namespace

auto evaluate(
    const Position& position,
    PawnTable*      pawnTable)
    -> Score
{
    // attack sets are calculated once & shared between the mobility, king zone, center control, space & threat terms
    mobility::Counter mobilityCounter { position };

    const auto attacks = chess::moves::compute_attacks(position, mobilityCounter);

    const auto control = detail::square_control(position, attacks);

    const auto total = score_pieces(position)
                     + no_pieces_left_bonus(position)
                     + score_rook_files(position)
                     + score_connected_rooks(position)
                     + score_squares_controlled_around_kings(position, attacks, control)
                     + detail::score_positional(position, control)
                     + detail::score_pawn_structure(position, pawnTable)
                     + score_king_safety(position)
                     + score_mobility(position, mobilityCounter)
                     + detail::score_threats(position, attacks)
                     + score_endgame_mopup(position);

    return Score { static_cast<Value>(total.blend(middlegame_phase(position), MAX_PHASE)) };
}

} // namespace ben_bot::eval
