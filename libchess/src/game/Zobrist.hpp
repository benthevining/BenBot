/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace chess::game::zobrist {

using board::Pieces;
using board::Square;
using pieces::Color;
using PieceType = pieces::Type;
using Value     = std::uint64_t;

[[nodiscard, gnu::const]] Value calculate(
    Color                 sideToMove,
    const Pieces&         whitePieces,
    const Pieces&         blackPieces,
    const CastlingRights& whiteRights,
    const CastlingRights& blackRights,
    std::optional<Square> enPassantTargetSquare);

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
