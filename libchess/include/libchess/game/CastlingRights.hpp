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
    This file defines the CastlingRights struct.
    @ingroup game
 */

#pragma once

#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::game {

using board::File;
using board::Rank;
using moves::Move;
using pieces::Color;
using PieceType = pieces::Type;

/** This struct encapsulates one side's castling rights.

    @ingroup game
 */
struct CastlingRights final {
    /** True if kingside (or "short") castling is available. */
    bool kingside { true };

    /** True if queenside (or "long") castling is available. */
    bool queenside { true };

    /** Returns true if the two sets of castling rights are identical. */
    [[nodiscard]] constexpr bool operator==(const CastlingRights&) const noexcept = default;

    /** Called when the king moves.
        Moving the king loses castling rights for both directions.
     */
    constexpr void king_moved() noexcept;

    /** Called when a rook moves.
        Moving a rook loses castling rights to that side.

        @param isKingside Should be true if the move's ``from`` square is on the kingside.
     */
    constexpr void rook_moved(bool isKingside) noexcept;

    /** Call this any time a move is made by this side to update the castling rights. */
    constexpr void our_move(const Move& move) noexcept;

    /** Call this any time the opponent makes a move to update the castling rights.
        An opponent's move can remove castling rights if they capture a rook.

        @tparam Side The color that this castling rights object represents.
     */
    template <Color Side>
    constexpr void their_move(const Move& move) noexcept;

    /** Returns true if castling either direction is possible. */
    [[nodiscard]] constexpr bool either() const noexcept { return kingside or queenside; }

    /** Returns true if neither castling direction is available. */
    [[nodiscard]] constexpr bool neither() const noexcept { return not either(); }
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

constexpr void CastlingRights::king_moved() noexcept
{
    kingside  = false;
    queenside = false;
}

constexpr void CastlingRights::rook_moved(const bool isKingside) noexcept
{
    // if the rook wasn't already on its starting square, then the castling rights
    // to that side would've already been lost, so we can tell if any rook move
    // should lose castling rights to that side with a simple boolean indicating
    // if the move's from square is on the kingside

    kingside  = kingside and not isKingside;
    queenside = queenside and isKingside;
}

constexpr void CastlingRights::our_move(const Move& move) noexcept
{
    switch (move.piece) {
        case PieceType::King: king_moved(); break;
        case PieceType::Rook: rook_moved(move.from.is_kingside()); break;
        default             : break;
    }
}

template <Color Side>
constexpr void CastlingRights::their_move(const Move& move) noexcept
{
    // we want to mark castling rights as lost when a rook is captured
    // we simply test if the move's to square is the rook's starting position,
    // since the rook either must've been there (in which case this move is a
    // rook capture), or must've already moved (meaning the rights to that side
    // must've already been lost)

    static constexpr auto backRank = Side == Color::White ? Rank::One : Rank::Eight;

    if (move.to.rank != backRank)
        return;

    kingside  = kingside and (move.to.file != File::H);
    queenside = queenside and (move.to.file != File::A);
}

} // namespace chess::game
