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
    This file defines the PieceSquareTables class.
    @ingroup eval
 */

#pragma once

#include <array>
#include <libchess/board/BitboardIndex.hpp>
#include <string>
#include <string_view>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace ben_bot::eval {

using chess::game::Position;

/** This struct encapsulates piece square table data, and provides
    methods for loading and saving the data in JSON format.
    We currently have one table per piece type, with different middle-
    and endgame tables for the king only.

    @ingroup eval
 */
struct PieceSquareTables final {
    /** Typedef for a single piece table. */
    using Table = std::array<int, chess::board::NUM_SQUARES>;

    /** Pawn values. */
    Table pawn;

    /** Knight values. */
    Table knight;

    /** Bishop values. */
    Table bishop;

    /** Rook values. */
    Table rook;

    /** Queen values. */
    Table queen;

    /** King middlegame values. */
    Table kingMiddlegame;

    /** King endgame values. */
    Table kingEndgame;

    /** Scores piece placement using the current PST data. */
    [[nodiscard]] auto score_piece_placement(
        const Position& position, float endgameWeight) const
        -> int;

    /** Serializes the current PST data to a JSON string. */
    [[nodiscard]] auto to_string() const -> std::string;

    /** Restores PST data from a serialized JSON string. */
    [[nodiscard]] static auto from_string(std::string_view text) -> PieceSquareTables;

    /** Returns the default PST data. */
    [[nodiscard]] static auto get_default() -> PieceSquareTables;
};

} // namespace ben_bot::eval
