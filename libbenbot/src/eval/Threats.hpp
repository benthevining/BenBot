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
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/AttackInfo.hpp>
#include <libchess/pieces/Colors.hpp>

namespace ben_bot::eval::detail {

using chess::game::Position;
using chess::moves::AttackInfo;

// The threat weights are Term::PawnThreat, MinorOnRook, MinorOnQueen, RookOnQueen & Hanging
// in DefaultWeights.hpp. Why they have the values they do:
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

template <typename Tracer>
[[nodiscard]] auto threats_for_side(
    const AttackInfo::Side& ourAttacks, const AttackInfo::Side& theirAttacks,
    const chess::board::Pieces& theirPieces, const chess::pieces::Color side, Tracer& tracer) noexcept
    -> TaperedScore
{
    const auto targets = theirPieces.knights | theirPieces.bishops | theirPieces.rooks | theirPieces.queens;

    const auto minors = ourAttacks.knights | ourAttacks.bishops;

    // A piece or pawn is strongly protected if one of their pawns defends it, or if they attack
    // its square twice and we don't. It's weak if we attack it and it isn't strongly protected.
    // A weak target is hanging if they don't defend it at all, or if it's a piece (not a pawn)
    // that we attack twice.
    const auto stronglyProtected = theirAttacks.pawns | (theirAttacks.twice & ourAttacks.twice.inverse());

    const auto weak = (targets | theirPieces.pawns) & stronglyProtected.inverse() & ourAttacks.all;

    const auto hanging = weak & (theirAttacks.all.inverse() | (targets & ourAttacks.twice));

    // Each term counts the pieces it hits once, even if several of our pieces attack them.
    // The terms themselves stack, so a rook attacked by a pawn & a knight gets both bonuses,
    // as in Stockfish & Weiss.
    const auto pawnThreats  = static_cast<int>((ourAttacks.pawns & targets).count());
    const auto minorOnRook  = static_cast<int>((minors & theirPieces.rooks).count());
    const auto minorOnQueen = static_cast<int>((minors & theirPieces.queens).count());
    const auto rookOnQueen  = static_cast<int>((ourAttacks.rooks & theirPieces.queens).count());
    const auto numHanging   = static_cast<int>(hanging.count());

    tracer.add(Term::PawnThreat, side, pawnThreats);
    tracer.add(Term::MinorOnRook, side, minorOnRook);
    tracer.add(Term::MinorOnQueen, side, minorOnQueen);
    tracer.add(Term::RookOnQueen, side, rookOnQueen);
    tracer.add(Term::Hanging, side, numHanging);

    return DEFAULT_WEIGHTS.get(Term::PawnThreat) * pawnThreats
         + DEFAULT_WEIGHTS.get(Term::MinorOnRook) * minorOnRook
         + DEFAULT_WEIGHTS.get(Term::MinorOnQueen) * minorOnQueen
         + DEFAULT_WEIGHTS.get(Term::RookOnQueen) * rookOnQueen
         + DEFAULT_WEIGHTS.get(Term::Hanging) * numHanging;
}

// bonus for attacking enemy pieces. Each side's threats count the same whoever's turn it is,
// and the result is from the side to move's point of view, like the other eval terms.
template <typename Tracer>
[[nodiscard]] auto score_threats(
    const Position& position, const AttackInfo& attacks, Tracer& tracer) noexcept
    -> TaperedScore
{
    using chess::pieces::Color;

    const auto white = threats_for_side(attacks.white, attacks.black, position.blackPieces, Color::White, tracer);
    const auto black = threats_for_side(attacks.black, attacks.white, position.whitePieces, Color::Black, tracer);

    if (position.is_white_to_move())
        return white - black;

    return black - white;
}

} // namespace ben_bot::eval::detail
