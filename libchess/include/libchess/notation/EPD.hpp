/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for working with EPD notation.
    @ingroup notation
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

namespace chess::notation {

using game::Position;
using std::string;

/** A position combined with optional EPD opcodes.

    @ingroup notation
 */
struct EPDPosition final {
    /** The position itself. */
    Position position {};

    /** This position's EPD operations.
        Values should not be quoted.
     */
    std::unordered_map<string, string> operations;
};

/** Parses an EPD string.

    @throws std::invalid_argument An exception will be thrown if the
    EPD string cannot be parsed successfully.

    @ingroup notation
    @relates EPDPosition
 */
[[nodiscard]] EPDPosition from_epd(std::string_view epdString);

/** Writes a position to an EPD string.

    @ingroup notation
    @relates EPDPosition
 */
[[nodiscard]] string to_epd(const EPDPosition& pos);

} // namespace chess::notation
