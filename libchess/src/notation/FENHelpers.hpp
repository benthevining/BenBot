/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

// This file provides functions common to FEN and EPD handling

#pragma once

#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Position.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace chess::notation::fen_helpers {

using board::Square;
using game::Position;

void write_piece_positions(
    const Position& position,
    std::string&    output);

void write_castling_rights(
    const game::CastlingRights& whiteRights,
    const game::CastlingRights& blackRights,
    std::string&                output);

void write_en_passant_target_square(
    std::optional<Square> targetSquare,
    std::string&          output);

void parse_piece_positions(
    std::string_view fenFragment, Position& position);

void parse_side_to_move(
    std::string_view fenFragment, Position& position);

void parse_castling_rights(
    std::string_view fenFragment, Position& position);

void parse_en_passant_target_square(
    std::string_view fenFragment, Position& position);

} // namespace chess::notation::fen_helpers
