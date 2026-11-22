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
#include <cstdint>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libbenbot/eval/Trace.hpp>
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

    // the game phase is the sum of the phase weights of the non-pawn pieces left on the board
    // (knights & bishops 1, rooks 2, queens 4, see Trace.hpp). Returns a [0..MAX_PHASE] value that
    // is MAX_PHASE in the starting position, and falls to 0 as the pieces are traded off. It's capped
    // at MAX_PHASE, for positions with promoted pieces
    [[nodiscard, gnu::pure]] constexpr auto middlegame_phase(
        const Position& position) noexcept
        -> std::int32_t
    {
        const auto side_phase = [](const chess::board::Pieces& pieces) {
            return (static_cast<std::int32_t>(pieces.knights.count()) * phase_weights::KNIGHT)
                 + (static_cast<std::int32_t>(pieces.bishops.count()) * phase_weights::BISHOP)
                 + (static_cast<std::int32_t>(pieces.rooks.count()) * phase_weights::ROOK)
                 + (static_cast<std::int32_t>(pieces.queens.count()) * phase_weights::QUEEN);
        };

        return std::min(side_phase(position.whitePieces) + side_phase(position.blackPieces), MAX_PHASE);
    }

    using chess::board::Bitboard;
    using chess::board::Pieces;
    using chess::pieces::Color;

    namespace masks = chess::board::masks;

    [[nodiscard, gnu::const]] constexpr auto opposite(const Color side) noexcept -> Color
    {
        return side == Color::White ? Color::Black : Color::White;
    }

    // The scoring functions below take a tracer, and tell it how many times each weight applied
    // for each side. The engine's eval uses the NullTracer, which does nothing.

    // the material & piece square table terms for one side, from that side's point of view
    template <bool IsBlack, typename Tracer>
    [[nodiscard]] auto score_side_pieces(
        const Pieces& pieces, Tracer& tracer) noexcept
        -> TaperedScore
    {
        static constexpr auto Side = IsBlack ? Color::Black : Color::White;

        TaperedScore score;

        for (const auto type : { PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
                 PieceType::Rook, PieceType::Queen, PieceType::King }) {
            auto board = pieces.get_type(type);

            if (type != PieceType::King) {
                const auto count = static_cast<int>(board.count());

                score += DEFAULT_WEIGHTS.material(type) * count;

                tracer.add(Term::Material, Side, count, static_cast<size_t>(type));
            }

            // the tables are from White's point of view
            if constexpr (IsBlack) {
                board = chess::board::flips::vertical(board);
            }

            const auto pst = DEFAULT_WEIGHTS.pst(type);

            for (const auto idx : board.indices()) {
                assert(idx < pst.size());
                score += pst[idx];

                tracer.add(Term::Pst, Side, 1, (static_cast<size_t>(type) * chess::board::NUM_SQUARES) + idx);
            }
        }

        return score;
    }

    // material & piece placement
    template <typename Tracer>
    [[nodiscard]] auto score_pieces(
        const Position& position, Tracer& tracer) noexcept
        -> TaperedScore
    {
        if (position.is_black_to_move()) {
            return score_side_pieces<true>(position.our_pieces(), tracer)
                 - score_side_pieces<false>(position.their_pieces(), tracer);
        }

        return score_side_pieces<false>(position.our_pieces(), tracer)
             - score_side_pieces<true>(position.their_pieces(), tracer);
    }

    // awards a bonus for rooks on open or half-open files
    template <typename Tracer>
    [[nodiscard]] auto score_rook_files(
        const Position& position, Tracer& tracer) noexcept
        -> TaperedScore
    {
        auto score_side_rooks = [&position, &tracer](const Pieces& pieces, const Color side) {
            TaperedScore score;

            for (const auto square : pieces.rooks.squares()) {
                if (position.is_file_half_open(square.file)) {
                    score += DEFAULT_WEIGHTS.get(Term::RookHalfOpenFile);
                    tracer.add(Term::RookHalfOpenFile, side, 1);
                } else if (position.is_file_open(square.file)) {
                    score += DEFAULT_WEIGHTS.get(Term::RookOpenFile);
                    tracer.add(Term::RookOpenFile, side, 1);
                }
            }

            return score;
        };

        return score_side_rooks(position.our_pieces(), position.sideToMove)
             - score_side_rooks(position.their_pieces(), opposite(position.sideToMove));
    }

    // bonus for connected rooks
    template <typename Tracer>
    [[nodiscard]] auto score_connected_rooks(
        const Position& position, Tracer& tracer) noexcept
        -> TaperedScore
    {
        auto score_connected_rooks = [&position, &tracer](const Pieces& pieces, const Color side) {
            TaperedScore score;

            // I originally tried adding a smaller bonus for connected rooks on any rank or file,
            // but experimental results showed that the engine was stronger with just the bonus
            // for connected rooks on open files

            for (const auto file : position.get_open_files()) {
                if (const auto mask = masks::files::get(file);
                    (mask & pieces.rooks).count() > 1uz) {
                    score += DEFAULT_WEIGHTS.get(Term::ConnectedRooksOpenFile);
                    tracer.add(Term::ConnectedRooksOpenFile, side, 1);
                }
            }

            return score;
        };

        return score_connected_rooks(position.our_pieces(), position.sideToMove)
             - score_connected_rooks(position.their_pieces(), opposite(position.sideToMove));
    }

    // awards various penalties for king danger
    template <typename Tracer>
    [[nodiscard]] auto score_king_safety(
        const Position& position, Tracer& tracer) noexcept
        -> TaperedScore
    {
        auto score_side_king = [&position, &tracer,
                                   allPawns = position.whitePieces.pawns | position.blackPieces.pawns](
                                   const Pieces& pieces, const chess::game::CastlingRights& castlingRights,
                                   const Pieces& enemyPieces, const Color side) {
            // These terms aren't scaled by the opponent's material: each one has its own middlegame &
            // endgame weight, so the game phase blend already decides how much they count as pieces
            // are traded off

            TaperedScore score;

            const auto location = pieces.get_king_location();

            // king on open file or diagonal
            if (position.is_file_half_open(location.file)) {
                score += DEFAULT_WEIGHTS.get(Term::KingHalfOpenFile);
                tracer.add(Term::KingHalfOpenFile, side, 1);
            } else if (position.is_file_open(location.file)
                       or (masks::diagonal(location) & allPawns).none()
                       or (masks::antidiagonal(location) & allPawns).none()) {
                score += DEFAULT_WEIGHTS.get(Term::KingOpenLine);
                tracer.add(Term::KingOpenLine, side, 1);
            }

            using chess::board::File;

            // king stranded in center without castling rights
            if (castlingRights.neither()
                and (location.file == File::D or location.file == File::E)) {
                score += DEFAULT_WEIGHTS.get(Term::KingStranded);
                tracer.add(Term::KingStranded, side, 1);
            }

            // enemy knights & queens near king

            for (const auto knightPos : enemyPieces.knights.squares()) {
                static constexpr auto MAX_DISTANCE = 6uz;

                const auto distance = static_cast<int>(MAX_DISTANCE - knight_distance(location, knightPos));

                // penalty increases with smaller distance
                score += DEFAULT_WEIGHTS.get(Term::KnightTropism) * distance;
                tracer.add(Term::KnightTropism, side, distance);
            }

            for (const auto queenPos : enemyPieces.queens.squares()) {
                static constexpr auto MAX_DISTANCE = 7uz;

                const auto distance = static_cast<int>(MAX_DISTANCE - chebyshev_distance(location, queenPos));

                // penalty increases with smaller distance
                score += DEFAULT_WEIGHTS.get(Term::QueenTropism) * distance;
                tracer.add(Term::QueenTropism, side, distance);
            }

            return score;
        };

        const bool isWhite = position.is_white_to_move();

        const auto ourScore = score_side_king(
            position.our_pieces(),
            isWhite ? position.whiteCastlingRights : position.blackCastlingRights,
            position.their_pieces(),
            position.sideToMove);

        const auto theirScore = score_side_king(
            position.their_pieces(),
            isWhite ? position.blackCastlingRights : position.whiteCastlingRights,
            position.our_pieces(),
            opposite(position.sideToMove));

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
        template <typename Tracer>
        struct Counter final {
            Counter(const Position& position, Tracer& tracerToUse) noexcept
                : whitePieces { position.whitePieces.occupied }
                , blackPieces { position.blackPieces.occupied }
                , tracer { tracerToUse }
            {
            }

            Bitboard whitePieces;
            Bitboard blackPieces;

            TaperedScore white;
            TaperedScore black;

            Tracer& tracer;

            void operator()(
                const Color side, const PieceType type, const Bitboard attacks, const AttackInfo& info) noexcept
            {
                const bool isWhite = side == Color::White;

                const auto friendly         = isWhite ? whitePieces : blackPieces;
                const auto enemyPawnAttacks = isWhite ? info.black.pawns : info.white.pawns;

                const auto mobilityArea = (friendly | enemyPawnAttacks).inverse();

                const auto numMoves = (attacks & mobilityArea).count();

                (isWhite ? white : black) += DEFAULT_WEIGHTS.get(term_for(type), numMoves);

                tracer.add(term_for(type), side, 1, numMoves);
            }
        };

    } // namespace mobility

    // bonus for pieces with more available moves
    template <typename Tracer>
    [[nodiscard, gnu::pure]] auto score_mobility(
        const Position& position, const mobility::Counter<Tracer>& counter) noexcept
        -> TaperedScore
    {
        if (position.is_white_to_move())
            return counter.white - counter.black;

        return counter.black - counter.white;
    }

    template <typename Tracer>
    [[nodiscard]] auto score_squares_controlled_around_kings(
        const Position& position, const AttackInfo& attacks, const detail::SquareControl& control,
        Tracer& tracer) noexcept
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

        tracer.add(Term::KingAreaControl, Color::White, whiteScore);
        tracer.add(Term::KingAreaControl, Color::Black, blackScore);

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

    // The mop-up & last piece terms are endgame conversion helpers that aren't tunable, so
    // the tracer gets them as a fixed part of the eval rather than by weight.
    template <typename Tracer>
    [[nodiscard]] auto score_endgame_helpers(
        const Position& position, Tracer& tracer) noexcept
        -> TaperedScore
    {
        const auto score = no_pieces_left_bonus(position) + score_endgame_mopup(position);

        tracer.add_base(position.sideToMove, score);

        return score;
    }

    // the eval, from the side to move's point of view
    template <typename Tracer>
    [[nodiscard]] auto evaluate_impl(
        const Position& position,
        PawnTable*      pawnTable,
        Tracer&         tracer)
        -> Score
    {
        // attack sets are calculated once & shared between the mobility, king zone, center control, space & threat terms
        mobility::Counter<Tracer> mobilityCounter { position, tracer };

        const auto attacks = chess::moves::compute_attacks(position, mobilityCounter);

        const auto control = detail::square_control(position, attacks);

        const auto total = score_pieces(position, tracer)
                         + score_endgame_helpers(position, tracer)
                         + score_rook_files(position, tracer)
                         + score_connected_rooks(position, tracer)
                         + score_squares_controlled_around_kings(position, attacks, control, tracer)
                         + detail::score_positional(position, control, tracer)
                         + detail::score_pawn_structure(position, pawnTable, tracer)
                         + score_king_safety(position, tracer)
                         + score_mobility(position, mobilityCounter)
                         + detail::score_threats(position, attacks, tracer);

        return Score { static_cast<Value>(total.blend(middlegame_phase(position), MAX_PHASE)) };
    }

} // namespace

auto evaluate(
    const Position& position,
    PawnTable*      pawnTable)
    -> Score
{
    NullTracer tracer;

    return evaluate_impl(position, pawnTable, tracer);
}

auto evaluate_traced(const Position& position) -> CoeffTracer
{
    CoeffTracer tracer;

    tracer.phase      = middlegame_phase(position);
    tracer.sideToMove = position.sideToMove;

    // the traced eval never uses the pawn table, so that the tracer sees every pawn term
    [[maybe_unused]] const auto score = evaluate_impl(position, nullptr, tracer);

    assert(tracer.score(DEFAULT_WEIGHTS) == score);

    return tracer;
}

} // namespace ben_bot::eval
