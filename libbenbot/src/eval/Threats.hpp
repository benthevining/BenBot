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

#pragma once

#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/AttackInfo.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

namespace ben_bot::eval::detail {

using chess::game::Position;
using chess::moves::AttackInfo;
using chess::pieces::Color;

// The threat weights are Term::PawnThreat, MinorOnRook, MinorOnQueen, RookOnQueen, Hanging &
// PushThreat in DefaultWeights.hpp. Why they have the values they do:
//
// PawnThreat (60, 30):
//   bonus for each enemy knight, bishop, rook or queen attacked by one of our pawns
//   (the king isn't a target, since checks are handled by the search)
//
// MinorOnRook (50, 30):
//   bonus for each enemy rook attacked by one of our knights or bishops
//   (below Stockfish's 70/56, Weiss's 64/5 & Ethereal's 59/19, since we have no tuner yet)
//
// MinorOnQueen (45, 20):
//   bonus for each enemy queen attacked by one of our knights or bishops. The queen can usually
//   just move away, and in the endgame the threat often only forces a queen trade, so the endgame
//   value is kept below the middlegame one (Weiss & Ethereal score it near zero)
//
// RookOnQueen (40, 15):
//   bonus for each enemy queen attacked by one of our rooks. A rook attacking a queen is often
//   defended by it or traded off, so this is the smallest of the piece threats
//
// Hanging (35, 15):
//   bonus for each enemy piece or pawn (not the king) that we attack and that's hanging: either
//   they don't defend it at all, or it's a piece that we attack twice and they don't defend
//   strongly enough. The middlegame value is about two-thirds of Stockfish's 54, since when
//   we're to move the quiescence search will often just take it, so this partly double-counts
//   the capture. The endgame value is close to Stockfish's 17, since in the endgame there are
//   fewer other pieces to tie down, so a hanging piece can usually just move away.
//
// PushThreat:
//   bonus for each enemy knight, bishop, rook or queen that one of our pawns could attack by
//   making a safe push. This is a threat for the next move, which the enemy can often answer
//   by moving the piece or stopping the push (Stockfish 38/18, Weiss 24/3, Ethereal 18/22)

template <Color Side, typename Tracer>
[[nodiscard]] auto threats_for_side(
    const Position& position, const AttackInfo& attacks, Tracer& tracer) noexcept
    -> TaperedScore
{
    static constexpr auto OtherSide = chess::pieces::other_side<Side>();

    // the rank a pawn lands on after a single push from its starting rank
    static constexpr auto thirdRank = Side == Color::White
                                        ? chess::board::masks::ranks::THREE
                                        : chess::board::masks::ranks::SIX;

    const auto& ourAttacks   = attacks.side<Side>();
    const auto& theirAttacks = attacks.side<OtherSide>();
    const auto& theirPieces  = position.pieces_for<OtherSide>();

    const auto targets = theirPieces.knights | theirPieces.bishops | theirPieces.rooks | theirPieces.queens;

    const auto minors = ourAttacks.knights | ourAttacks.bishops;

    // A piece or pawn is strongly protected if one of their pawns defends it, or if they attack
    // its square twice and we don't. It's weak if we attack it and it isn't strongly protected.
    // A weak target is hanging if they don't defend it at all, or if it's a piece (not a pawn)
    // that we attack twice.
    const auto stronglyProtected = theirAttacks.pawns | (theirAttacks.twice & ourAttacks.twice.inverse());

    const auto weak = (targets | theirPieces.pawns) & stronglyProtected.inverse() & ourAttacks.all;

    const auto hanging = weak & (theirAttacks.all.inverse() | (targets & ourAttacks.twice));

    // Squares our pawns can push to (including double pushes through an empty square), that
    // no enemy pawn attacks, and that we either defend or they don't attack at all
    namespace pseudo_legal = chess::moves::pseudo_legal;

    const auto empty = position.free();

    auto pushes = pseudo_legal::pawn_pushes<Side>(position.pieces_for<Side>().pawns, empty);
    pushes |= pseudo_legal::pawn_pushes<Side>(pushes & thirdRank, empty);
    pushes &= theirAttacks.pawns.inverse() & (ourAttacks.all | theirAttacks.all.inverse());

    // pieces already attacked by one of our pawns are scored by PawnThreat instead
    const auto pushThreats = chess::moves::patterns::pawn_attacks<Side>(pushes)
                           & targets & ourAttacks.pawns.inverse();

    // Each term counts the pieces it hits once, even if several of our pieces attack them.
    // The terms themselves stack, so a rook attacked by a pawn & a knight gets both bonuses,
    // as in Stockfish & Weiss.
    const auto pawnThreats  = static_cast<int>((ourAttacks.pawns & targets).count());
    const auto minorOnRook  = static_cast<int>((minors & theirPieces.rooks).count());
    const auto minorOnQueen = static_cast<int>((minors & theirPieces.queens).count());
    const auto rookOnQueen  = static_cast<int>((ourAttacks.rooks & theirPieces.queens).count());
    const auto numHanging   = static_cast<int>(hanging.count());
    const auto numPush      = static_cast<int>(pushThreats.count());

    tracer.add(Term::PawnThreat, Side, pawnThreats);
    tracer.add(Term::MinorOnRook, Side, minorOnRook);
    tracer.add(Term::MinorOnQueen, Side, minorOnQueen);
    tracer.add(Term::RookOnQueen, Side, rookOnQueen);
    tracer.add(Term::Hanging, Side, numHanging);
    tracer.add(Term::PushThreat, Side, numPush);

    return DEFAULT_WEIGHTS.get(Term::PawnThreat) * pawnThreats
         + DEFAULT_WEIGHTS.get(Term::MinorOnRook) * minorOnRook
         + DEFAULT_WEIGHTS.get(Term::MinorOnQueen) * minorOnQueen
         + DEFAULT_WEIGHTS.get(Term::RookOnQueen) * rookOnQueen
         + DEFAULT_WEIGHTS.get(Term::Hanging) * numHanging
         + DEFAULT_WEIGHTS.get(Term::PushThreat) * numPush;
}

// bonus for attacking enemy pieces. Each side's threats count the same whoever's turn it is,
// and the result is from the side to move's point of view, like the other eval terms.
template <typename Tracer>
[[nodiscard]] auto score_threats(
    const Position& position, const AttackInfo& attacks, Tracer& tracer) noexcept
    -> TaperedScore
{
    const auto white = threats_for_side<Color::White>(position, attacks, tracer);
    const auto black = threats_for_side<Color::Black>(position, attacks, tracer);

    if (position.is_white_to_move())
        return white - black;

    return black - white;
}

} // namespace ben_bot::eval::detail
