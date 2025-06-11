/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides hash keys used for Zobrist hashing.
    @ingroup game
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/board/File.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>

namespace chess::game {
struct Position;
} // namespace chess::game

/** This namespace contains hash keys used for Zobrist hashing.
    These functions are mainly for internal usage by the Position class.
    @ingroup game
 */
namespace chess::game::zobrist {

using board::File;
using board::Pieces;
using board::Square;
using pieces::Color;
using PieceType = pieces::Type;
using Value     = std::uint64_t;

static constexpr Value BLACK_TO_MOVE { 0x679ebe6f2ed869a4ULL };

static constexpr Value WHITE_KINGSIDE_CASTLE { 0x6b63254b15e00a87ULL };
static constexpr Value WHITE_QUEENSIDE_CASTLE { 0x098dc1575ddbd151ULL };
static constexpr Value BLACK_KINGSIDE_CASTLE { 0xdbb675f686df04a9ULL };
static constexpr Value BLACK_QUEENSIDE_CASTLE { 0x71588a053b2bd9e5ULL };

[[nodiscard, gnu::const]] Value en_passant_key(File file);

[[nodiscard, gnu::const]] Value piece_key(
    PieceType type, Color side, const Square& square);

[[nodiscard, gnu::const]] Value calculate(
    Color                 sideToMove,
    const Pieces&         whitePieces,
    const Pieces&         blackPieces,
    const CastlingRights& whiteRights,
    const CastlingRights& blackRights,
    std::optional<Square> enPassantTargetSquare);

// each of these bools are true if the given right has changed since the last move
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
