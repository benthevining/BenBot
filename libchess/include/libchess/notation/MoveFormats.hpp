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
    This file provides an abstraction for formatting moves using any of
    the available notation types.
    @ingroup notation
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <expected>
#include <libchess/moves/Move.hpp>
#include <string>
#include <string_view>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::notation {

using game::Position;
using moves::Move;

/** This enum describes the supported notation formats for moves.
    @ingroup notation
    @see format_move()
 */
enum class MoveFormat : std::uint_least8_t {
    Algebraic, ///< Algebraic notation.
    ICCF,      ///< ICCF numeric notation.
    UCI        ///< UCI notation.
};

/** Prints a move using the given notation format.
    @ingroup notation
    @see MoveFormat, parse_move()
 */
[[nodiscard]] auto format_move(
    MoveFormat      format,
    const Position& position,
    Move            move) -> std::string;

/** Parses a move from a string using the given notation format.
    @ingroup notation
    @see MoveFormat, format_move()
 */
[[nodiscard]] auto parse_move(
    MoveFormat       format,
    const Position&  position,
    std::string_view string)
    -> std::expected<Move, std::string>;

} // namespace chess::notation
