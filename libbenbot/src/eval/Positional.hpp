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

#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/pieces/Colors.hpp>

namespace ben_bot::eval::detail {

using chess::game::Position;
using chess::pieces::Color;

namespace masks = chess::board::masks;
namespace moves = chess::moves;

[[nodiscard, gnu::const]] inline int score_center_control(
    const Position& position) noexcept
{
    const auto whiteControls = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, masks::CENTER, position.blackPieces.occupied, false));
    const auto blackControls = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, masks::CENTER, position.whitePieces.occupied, false));

    const bool isWhite = position.is_white_to_move();

    const auto ourControl   = isWhite ? whiteControls : blackControls;
    const auto theirControl = isWhite ? blackControls : whiteControls;

    return ourControl - theirControl;
}

// a crude way to evaluate "space":
// we take each side's pawn rearfill, and look at how many more of those squares are controlled by that side than by their opponent
// this serves to discourage the engine from overextending, but also to incentivize expanding the pawn line to claim more space
[[nodiscard, gnu::const]] inline int score_space(
    const Position& position) noexcept
{
    const auto behindWhitePawns = chess::board::fills::pawn_rear<Color::White>(position.whitePieces.pawns);
    const auto behindBlackPawns = chess::board::fills::pawn_rear<Color::Black>(position.blackPieces.pawns);

    const auto whiteSquares
        = static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, behindWhitePawns, position.blackPieces.occupied))
        - static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, behindWhitePawns, position.whitePieces.occupied));

    const auto blackSquares
        = static_cast<int>(moves::num_squares_attacked<Color::Black>(position.blackPieces, behindBlackPawns, position.whitePieces.occupied))
        - static_cast<int>(moves::num_squares_attacked<Color::White>(position.whitePieces, behindBlackPawns, position.blackPieces.occupied));

    const bool isWhite = position.is_white_to_move();

    const auto ourSquares   = isWhite ? whiteSquares : blackSquares;
    const auto theirSquares = isWhite ? blackSquares : whiteSquares;

    return (ourSquares - theirSquares) * 2;
}

[[nodiscard, gnu::const]] inline int score_positional(const Position& position) noexcept
{
    return score_center_control(position)
         + score_space(position);
}

} // namespace ben_bot::eval::detail
