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

[[nodiscard, gnu::pure]] inline auto threats_for_side(
    const AttackInfo::Side& ourAttacks, const chess::board::Pieces& theirPieces) noexcept
    -> TaperedScore
{
    const auto targets = theirPieces.knights | theirPieces.bishops | theirPieces.rooks | theirPieces.queens;

    return PAWN_THREAT * static_cast<int>((ourAttacks.pawns & targets).count());
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
