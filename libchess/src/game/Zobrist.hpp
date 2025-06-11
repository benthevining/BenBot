/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/board/Square.hpp>
#include <optional>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::moves {
struct Move;
} // namespace chess::moves

namespace chess::game::zobrist {

using board::Square;
using moves::Move;

using Value = std::uint64_t;

[[nodiscard, gnu::const]] Value calculate(const Position& pos);

// each of these bools should be true if the given right has changed since the last move
struct CastlingRightsChanges final {
    bool whiteKingside { false };
    bool whiteQueenside { false };
    bool blackKingside { false };
    bool blackQueenside { false };
};

[[nodiscard, gnu::const]] Value update(
    const Position& pos, const Move& move,
    std::optional<Square>        newEPTarget,
    const CastlingRightsChanges& rightsChanges);

} // namespace chess::game::zobrist
