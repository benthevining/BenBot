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

#include <libbenbot/eval/TaperedScore.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/AttackInfo.hpp>

namespace ben_bot::eval::detail {

using chess::game::Position;
using chess::moves::AttackInfo;

// bonus for each enemy knight, bishop, rook or queen attacked by one of our pawns
// (the king isn't a target, since checks are handled by the search)
inline constexpr TaperedScore PAWN_THREAT { .mg = 60, .eg = 30 };

// bonus for each enemy rook attacked by one of our knights or bishops
// (below Stockfish's 70/56, Weiss's 64/5 & Ethereal's 59/19, since we have no tuner yet)
inline constexpr TaperedScore MINOR_ON_ROOK { .mg = 50, .eg = 30 };

// bonus for each enemy queen attacked by one of our knights or bishops. The queen can usually
// just move away, and in the endgame the threat often only forces a queen trade, so the endgame
// value is kept below the middlegame one (Weiss & Ethereal score it near zero)
inline constexpr TaperedScore MINOR_ON_QUEEN { .mg = 45, .eg = 20 };

// bonus for each enemy queen attacked by one of our rooks. A rook attacking a queen is often
// defended by it or traded off, so this is the smallest of the piece threats
inline constexpr TaperedScore ROOK_ON_QUEEN { .mg = 40, .eg = 15 };

// bonus for each enemy piece or pawn (not the king) that we attack and that's hanging: either
// they don't defend it at all, or it's a piece that we attack twice and they don't defend
// strongly enough. The middlegame value is about two-thirds of Stockfish's 54, since when
// we're to move the quiescence search will often just take it, so this partly double-counts
// the capture. The endgame value is close to Stockfish's 17, since in the endgame there are
// fewer other pieces to tie down, so a hanging piece can usually just move away.
inline constexpr TaperedScore HANGING { .mg = 35, .eg = 15 };

[[nodiscard, gnu::pure]] inline auto threats_for_side(
    const AttackInfo::Side& ourAttacks, const AttackInfo::Side& theirAttacks,
    const chess::board::Pieces& theirPieces) noexcept
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
    return PAWN_THREAT * static_cast<int>((ourAttacks.pawns & targets).count())
         + MINOR_ON_ROOK * static_cast<int>((minors & theirPieces.rooks).count())
         + MINOR_ON_QUEEN * static_cast<int>((minors & theirPieces.queens).count())
         + ROOK_ON_QUEEN * static_cast<int>((ourAttacks.rooks & theirPieces.queens).count())
         + HANGING * static_cast<int>(hanging.count());
}

// bonus for attacking enemy pieces. Each side's threats count the same whoever's turn it is,
// and the result is from the side to move's point of view, like the other eval terms.
[[nodiscard, gnu::pure]] inline auto score_threats(
    const Position& position, const AttackInfo& attacks) noexcept
    -> TaperedScore
{
    const auto white = threats_for_side(attacks.white, attacks.black, position.blackPieces);
    const auto black = threats_for_side(attacks.black, attacks.white, position.whitePieces);

    if (position.is_white_to_move())
        return white - black;

    return black - white;
}

} // namespace ben_bot::eval::detail
