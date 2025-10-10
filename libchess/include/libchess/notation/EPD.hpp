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

#include <expected>
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
struct [[nodiscard]] EPDPosition final {
    /** The position itself. */
    Position position {};

    /** This position's EPD operations.
        Values should not be quoted.
     */
    std::unordered_map<string, string> operations;

    /** Returns true if the two positions have the same Zobrist hash
        and an identical set of operations.
     */
    [[nodiscard]] auto operator==(const EPDPosition& other) const noexcept -> bool = default;
};

/** Parses an EPD string.

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup notation
    @relates EPDPosition
    @see parse_all_epds()
 */
[[nodiscard]] auto from_epd(std::string_view epdString)
    -> std::expected<EPDPosition, string>;

/** Parses all EPDs in a string containing one EPD per line.

    @ingroup notation
    @relates EPDPosition
    @see from_epd()
 */
[[nodiscard]] auto parse_all_epds(std::string_view fileContent)
    -> std::vector<EPDPosition>;

/** Writes a position to an EPD string.

    @ingroup notation
    @relates EPDPosition
 */
[[nodiscard]] auto to_epd(const EPDPosition& pos) -> string;

} // namespace chess::notation
