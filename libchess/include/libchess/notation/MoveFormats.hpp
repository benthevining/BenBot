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
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/ICCF.hpp>
#include <libchess/notation/UCI.hpp>
#include <string>
#include <utility> // IWYU pragma: keep - for std::unreachable()

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
    @see MoveFormat
 */
[[nodiscard]] inline auto format_move(
    const MoveFormat format,
    const Position&  position,
    const Move       move) -> std::string
{
    switch (format) {
        case MoveFormat::Algebraic:
            return to_alg(position, move);

        case MoveFormat::ICCF:
            return to_iccf(move);

        case MoveFormat::UCI:
            return to_uci(move);

        default:
            std::unreachable();
    }
}

} // namespace chess::notation
