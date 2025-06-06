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
#include <limits>

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
    has mate-in-1.
    @ingroup eval
 */
static constexpr auto MAX = std::numeric_limits<Value>::max();

/** The minimum possible evaluation score, i.e., if the side to move
    has gotten checkmated.
    @ingroup eval
 */
static constexpr auto MIN = std::numeric_limits<Value>::min();

/** A neutral, or draw, score.
    @ingroup eval
 */
static constexpr auto DRAW = static_cast<Value>(0);

/** Returns a numerical score representing the evaluation of the
    give position from the perspective of the side to move.
    Higher numbers are better for the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] Value evaluate(const Position& position);

} // namespace chess::eval
