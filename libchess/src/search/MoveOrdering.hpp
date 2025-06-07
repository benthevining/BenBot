/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <span>

namespace chess::search::detail {

using game::Position;
using moves::Move;

void order_moves_for_search(
    const Position& currentPosition,
    std::span<Move> moves);

} // namespace chess::search::detail
