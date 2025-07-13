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
    This file provides functions for working with EPD notation.
    @ingroup notation
 */

#pragma once

#include <libchess/game/Position.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
    @see parse_all_epds()
 */
[[nodiscard]] EPDPosition from_epd(std::string_view epdString);

/** Parses all EPDs in a string containing one EPD per line.

    @ingroup notation
    @relates EPDPosition
    @see from_epd()
 */
[[nodiscard]] std::vector<EPDPosition> parse_all_epds(std::string_view fileContent);

/** Writes a position to an EPD string.

    @ingroup notation
    @relates EPDPosition
 */
[[nodiscard]] string to_epd(const EPDPosition& pos);

} // namespace chess::notation
