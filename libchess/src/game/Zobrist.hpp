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

#pragma once

#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>

namespace chess::game::zobrist {

using board::Square;
using moves::Move;

[[nodiscard, gnu::pure]] auto calculate(const Position& pos) -> Position::Hash;

// each of these bools should be true if the given right has changed since the last move
struct [[nodiscard]] CastlingRightsChanges final {
    bool whiteKingside : 1 { false }; // cppcheck-suppress syntaxError
    bool whiteQueenside : 1 { false };
    bool blackKingside : 1 { false };
    bool blackQueenside : 1 { false };

    [[nodiscard]] auto update_hash(Position::Hash value) const noexcept -> Position::Hash;
};

[[nodiscard, gnu::pure]] auto update(
    const Position& pos, Move move,
    std::optional<Square> newEPTarget,
    CastlingRightsChanges rightsChanges)
    -> Position::Hash;

// calculates the hash of only the pawns in the position, ignoring all other state
[[nodiscard, gnu::pure]] auto calculate_pawns(const Position& pos) -> Position::Hash;

// returns the new pawn hash after making the given move. This must be called before
// the move is applied to the position
[[nodiscard, gnu::pure]] auto update_pawns(
    const Position& pos, Move move)
    -> Position::Hash;

[[nodiscard, gnu::pure]] auto after_null_move(
    const Position& pos)
    -> Position::Hash;

} // namespace chess::game::zobrist
