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

// bonus for each enemy knight, bishop, rook or queen that one of our pawns could attack by
// making a safe push. This is a threat for the next move, which the enemy can often answer
// by moving the piece or stopping the push, so it's well below PAWN_THREAT. It doesn't drop
// much in the endgame, since a pawn push gaining space with tempo still matters there
// (Stockfish 38/18, Weiss 24/3, Ethereal 18/22)
inline constexpr TaperedScore PUSH_THREAT { .mg = 20, .eg = 15 };

template <Color Side>
[[nodiscard, gnu::pure]] auto threats_for_side(
    const Position& position, const AttackInfo& attacks) noexcept
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

    // Squares our pawns can push to (including double pushes through an empty square), that
    // no enemy pawn attacks, and that we either defend or they don't attack at all
    namespace pseudo_legal = chess::moves::pseudo_legal;

    const auto empty = position.free();

    auto pushes = pseudo_legal::pawn_pushes<Side>(position.pieces_for<Side>().pawns, empty);
    pushes |= pseudo_legal::pawn_pushes<Side>(pushes & thirdRank, empty);
    pushes &= theirAttacks.pawns.inverse() & (ourAttacks.all | theirAttacks.all.inverse());

    // pieces already attacked by one of our pawns are scored by PAWN_THREAT instead
    const auto pushThreats = chess::moves::patterns::pawn_attacks<Side>(pushes)
                           & targets & ourAttacks.pawns.inverse();

    // Each term counts the pieces it hits once, even if several of our pieces attack them.
    // The terms themselves stack, so a rook attacked by a pawn & a knight gets both bonuses,
    // as in Stockfish & Weiss.
    return PAWN_THREAT * static_cast<int>((ourAttacks.pawns & targets).count())
         + MINOR_ON_ROOK * static_cast<int>((minors & theirPieces.rooks).count())
         + MINOR_ON_QUEEN * static_cast<int>((minors & theirPieces.queens).count())
         + ROOK_ON_QUEEN * static_cast<int>((ourAttacks.rooks & theirPieces.queens).count())
         + PUSH_THREAT * static_cast<int>(pushThreats.count());
}

// bonus for attacking enemy pieces. Each side's threats count the same whoever's turn it is,
// and the result is from the side to move's point of view, like the other eval terms.
[[nodiscard, gnu::pure]] inline auto score_threats(
    const Position& position, const AttackInfo& attacks) noexcept
    -> TaperedScore
{
    const auto white = threats_for_side<Color::White>(position, attacks);
    const auto black = threats_for_side<Color::Black>(position, attacks);

    if (position.is_white_to_move())
        return white - black;

    return black - white;
}

} // namespace ben_bot::eval::detail
