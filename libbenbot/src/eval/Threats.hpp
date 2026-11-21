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

[[nodiscard, gnu::pure]] inline auto threats_for_side(
    const AttackInfo::Side& ourAttacks, const chess::board::Pieces& theirPieces) noexcept
    -> TaperedScore
{
    const auto targets = theirPieces.knights | theirPieces.bishops | theirPieces.rooks | theirPieces.queens;

    const auto minors = ourAttacks.knights | ourAttacks.bishops;

    // Each term counts the pieces it hits once, even if several of our pieces attack them.
    // The terms themselves stack, so a rook attacked by a pawn & a knight gets both bonuses,
    // as in Stockfish & Weiss.
    return PAWN_THREAT * static_cast<int>((ourAttacks.pawns & targets).count())
         + MINOR_ON_ROOK * static_cast<int>((minors & theirPieces.rooks).count())
         + MINOR_ON_QUEEN * static_cast<int>((minors & theirPieces.queens).count())
         + ROOK_ON_QUEEN * static_cast<int>((ourAttacks.rooks & theirPieces.queens).count());
}

// bonus for attacking enemy pieces. Each side's threats count the same whoever's turn it is,
// and the result is from the side to move's point of view, like the other eval terms.
[[nodiscard, gnu::pure]] inline auto score_threats(
    const Position& position, const AttackInfo& attacks) noexcept
    -> TaperedScore
{
    const auto white = threats_for_side(attacks.white, position.blackPieces);
    const auto black = threats_for_side(attacks.black, position.whitePieces);

    if (position.is_white_to_move())
        return white - black;

    return black - white;
}

} // namespace ben_bot::eval::detail
