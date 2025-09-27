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

/** @defgroup notation Notation
    Functions for working with various types of chess game notation.

    @ingroup libchess
 */

/** @file
    This file provides functions for converting Position objects to
    and from FEN strings.

    @ingroup notation
 */

#pragma once

#include <expected>
#include <libchess/game/Position.hpp>
#include <string>
#include <string_view>

/** This namespace contains functions for working with various types of
    chess game notation.

    @ingroup notation
 */
namespace chess::notation {

using game::Position;

/** Returns the FEN representation of the given position.

    @param position The position to serialize as a FEN string.
    @param alwaysWriteEPSqare If true, the en passant square will be written
    any time the last move was a pawn double-advance; if false, the en passant
    square will only be written if the opponent has any pawns capable of
    capturing en passant (as in X-FEN).

    @ingroup notation
    @see from_fen()
 */
[[nodiscard]] std::string to_fen(
    const Position& position,
    bool            alwaysWriteEPSqare = true);

/** Returns a Position object encoding the given FEN string.

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup notation
    @see to_fen()
 */
[[nodiscard]] std::expected<Position, std::string> from_fen(std::string_view fenString);

} // namespace chess::notation
