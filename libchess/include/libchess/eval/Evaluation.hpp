/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup eval Evaluation
    Functions for evaluating positions.
 */

/** @file
    This file provides the top-level evaluation function.
    @ingroup eval
 */

#pragma once

#include <libchess/game/Position.hpp>

/** This namespace contains functions for evaluating positions.
    @ingroup eval
 */
namespace chess::eval {

using game::Position;

/** Floating-point type representing evaluation results.
    @ingroup eval
 */
using Value = double;

/** The maximum possible evaluation score, i.e., if the side to move
    has mate-in-1. If the side to move is in checkmate, the evaluation
    is ``-MATE``.

    @ingroup eval
 */
static constexpr auto MATE = static_cast<Value>(10000000);

/** A neutral, or draw, score.
    @ingroup eval
 */
static constexpr auto DRAW = static_cast<Value>(0);

/** Returns a numerical score representing the evaluation of the
    give position from the perspective of the side to move.
    Higher numbers are better for the side to move.

    The value returned is in the range ``[MIN, MAX]``.

    @ingroup eval
 */
[[nodiscard]] Value evaluate(const Position& position);

} // namespace chess::eval
