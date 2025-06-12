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
#include <optional>
#include <string>
#include <string_view>

namespace chess::game {
struct Position;
struct CastlingRights;
} // namespace chess::game

namespace chess::notation::fen_helpers {

using board::Square;
using game::Position;
using std::string_view;

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
    string_view fenFragment, Position& position);

void parse_side_to_move(
    string_view fenFragment, Position& position);

void parse_castling_rights(
    string_view fenFragment, Position& position);

void parse_en_passant_target_square(
    string_view fenFragment, Position& position);

} // namespace chess::notation::fen_helpers
