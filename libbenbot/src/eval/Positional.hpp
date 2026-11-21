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

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/AttackInfo.hpp>
#include <libchess/pieces/Colors.hpp>

namespace ben_bot::eval::detail {

using chess::board::Bitboard;
using chess::game::Position;
using chess::moves::AttackInfo;
using chess::pieces::Color;

namespace masks = chess::board::masks;

// The squares each side controls, as the center control, space & king zone terms count them.
// Pawn, knight & king attacks include squares occupied by friendly pieces, but slider attacks
// don't (the same squares that chess::moves::num_squares_attacked() counts).
struct SquareControl final {
    Bitboard white; // not including the king
    Bitboard black; // not including the king

    Bitboard whiteWithKing;
    Bitboard blackWithKing;
};

[[nodiscard, gnu::pure]] inline auto square_control(
    const Position& position, const AttackInfo& attacks) noexcept -> SquareControl
{
    auto side_control = [](const AttackInfo::Side& side, const chess::board::Pieces& pieces) {
        return side.pawns | side.knights
             | ((side.bishops | side.rooks | side.queens) & pieces.occupied.inverse());
    };

    const auto white = side_control(attacks.white, position.whitePieces);
    const auto black = side_control(attacks.black, position.blackPieces);

    return {
        .white         = white,
        .black         = black,
        .whiteWithKing = white | attacks.white.king,
        .blackWithKing = black | attacks.black.king
    };
}

[[nodiscard, gnu::pure]] inline auto score_center_control(
    const Position& position, const SquareControl& control) noexcept -> int
{
    const auto whiteControls = static_cast<int>((masks::CENTER & control.white).count());
    const auto blackControls = static_cast<int>((masks::CENTER & control.black).count());

    const bool isWhite = position.is_white_to_move();

    const auto ourControl   = isWhite ? whiteControls : blackControls;
    const auto theirControl = isWhite ? blackControls : whiteControls;

    return ourControl - theirControl;
}

// a crude way to evaluate "space":
// we take each side's pawn rearfill, and look at how many more of those squares are controlled by that side than by their opponent
// this serves to discourage the engine from overextending, but also to incentivize expanding the pawn line to claim more space
[[nodiscard, gnu::pure]] inline auto score_space(
    const Position& position, const SquareControl& control) noexcept -> int
{
    const auto behindWhitePawns = chess::board::fills::pawn_rear<Color::White>(position.whitePieces.pawns);
    const auto behindBlackPawns = chess::board::fills::pawn_rear<Color::Black>(position.blackPieces.pawns);

    const auto whiteSquares
        = static_cast<int>((behindWhitePawns & control.whiteWithKing).count())
        - static_cast<int>((behindWhitePawns & control.blackWithKing).count());

    const auto blackSquares
        = static_cast<int>((behindBlackPawns & control.blackWithKing).count())
        - static_cast<int>((behindBlackPawns & control.whiteWithKing).count());

    const bool isWhite = position.is_white_to_move();

    const auto ourSquares   = isWhite ? whiteSquares : blackSquares;
    const auto theirSquares = isWhite ? blackSquares : whiteSquares;

    return (ourSquares - theirSquares) * 2;
}

[[nodiscard, gnu::pure]] inline auto score_positional(
    const Position& position, const SquareControl& control) noexcept -> int
{
    return score_center_control(position, control)
         + score_space(position, control);
}

} // namespace ben_bot::eval::detail
