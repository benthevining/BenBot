/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines piece-square tables.
    @ingroup eval
 */

#pragma once

#include <libchess/game/Position.hpp>

namespace chess::eval {

using game::Position;

/** Returns an aggregate score for the placement of both sides' pieces,
    relative to the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] int score_piece_placement(const Position& position);

} // namespace chess::eval
