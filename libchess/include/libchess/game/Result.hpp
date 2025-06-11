/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the game result enum.
    @ingroup game
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t

namespace chess::game {

/** This enumeration defines possible game results.
    @ingroup game
 */
enum class Result : std::uint_least8_t {
    Draw,     ///< A draw result.
    WhiteWon, ///< A White win result.
    BlackWon  ///< A Black win result.
};

} // namespace chess::game
