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

/** @file
    This file provides a static exchange evaluation function.
    @ingroup eval
 */

#pragma once

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::moves {
struct Move;
} // namespace chess::moves

namespace ben_bot::eval {

using chess::game::Position;
using chess::moves::Move;

/** Static exchange evaluation (SEE).

    Returns the net material change for the side to move if it plays the given
    move, and then both sides continue recapturing on the move's destination
    square with their least valuable attacker, each side being free to stop
    recapturing whenever continuing would lose material. A positive value means
    the side to move wins material; a negative value means it loses material.

    This is computed using the swap algorithm described at
    https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm, and includes
    X-ray attackers (sliding pieces hidden behind other attackers of the same
    square). Promotions (including recaptures that promote) are assumed to be to
    a queen unless the move itself specifies otherwise. Pins and checks are not
    considered, so the result is an approximation.

    The move may also be a quiet move, in which case the result is zero if the
    moved piece can safely stand on its destination square, or negative if it
    can be won by the opponent. Castling moves always return zero.

    @ingroup eval
 */
[[nodiscard]] auto see(const Position& position, Move move) -> int;

} // namespace ben_bot::eval
