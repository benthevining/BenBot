/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup game Modeling of the chess game
    Classes to model the state of a game of chess.
 */

/** @file
    This file defines the Position class.
    @ingroup game
 */

#pragma once

#include <cstdint>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>

/** This namespace contains classes for modeling the state
    of a game of chess.

    @ingroup game
 */
namespace chess::game {

using board::File;
using pieces::Color;

/** This class models an instant in a game of chess.

    It describes where all the pieces are placed, as well as some
    additional metadata such as castling rights, side to move,
    en passant possibility, etc.

    @ingroup game

    @todo Detect threefold reps by keeping array<Position, 6> ?

    @todo Funcs to get passed pawns, backward pawns
    @todo Func to print board as ASCII/UTF8
    @todo Funcs is_stalemate(), is_checkmate(), is_check()

    @todo std::hash
 */
struct Position final {
    /** The positions of the White pieces. */
    board::Pieces whitePieces { Color::White };

    /** The positions of the Black pieces. */
    board::Pieces blackPieces { Color::Black };

    /** Indicates whose move it is in this position. */
    Color sideToMove { Color::White };

    /** Castling rights for the White player. */
    CastlingRights whiteCastlingRights;

    /** Castling rights for the Black player. */
    CastlingRights blackCastlingRights;

    /** If en passant is possible in this position, this holds
        the square that the pawn would land on after capturing
        en passant. If en passant is not possible, this will
        be ``nullopt``.
     */
    std::optional<board::Square> enPassantTargetSquare;

    /** This is a ply counter that enforces the 50-move rule.
        The counter s incremented after every move and reset by
        captures and pawn moves; if the counter reaches 100 and
        the side to move has at least 1 legal move, then the game
        is drawn.
     */
    std::uint_least8_t halfmoveClock { 0 };

    /** Returns true if the two positions are identical.
        @todo Exclude halfmove clock?
     */
    [[nodiscard]] constexpr bool operator==(const Position&) const noexcept = default;

    /** Returns a bitboard that is the union of all White and Black
        piece positions.
     */
    [[nodiscard]] constexpr board::Bitboard occupied() const noexcept
    {
        return whitePieces.occupied() | blackPieces.occupied();
    }

    /** Returns a bitboard that is the inverse of the ``occupied()`` board. */
    [[nodiscard]] constexpr board::Bitboard free() const noexcept
    {
        return occupied().inverse();
    }

    /** Returns true if there are no pawns of either color on the given file.
        @see get_open_files()
     */
    [[nodiscard]] constexpr bool is_file_open(const File file) const noexcept
    {
        return whitePieces.is_file_half_open(file) && blackPieces.is_file_half_open(file);
    }

    /** Returns an iterable range of File enumeration values corresponding
        to all open files in this position.

        @see is_file_open()
     */
    [[nodiscard]] constexpr auto get_open_files() const noexcept;

    /** Returns true if only one side has a pawn on the given file.
        @see get_half_open_files
     */
    [[nodiscard]] constexpr bool is_file_half_open(File file) const noexcept;

    /** Returns an iterable range of File enumeration values corresponding
        to all half-open files in this position.

        @see is_file_half_open()
     */
    [[nodiscard]] constexpr auto get_half_open_files() const noexcept;
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

constexpr auto Position::get_open_files() const noexcept
{
    return magic_enum::enum_values<File>()
         | std::views::filter([this](const File file) { return is_file_open(file); });
}

constexpr bool Position::is_file_half_open(const File file) const noexcept
{
    const bool whiteOpen = whitePieces.is_file_half_open(file);
    const bool blackOpen = blackPieces.is_file_half_open(file);

    // boolean XOR

    if (whiteOpen && blackOpen)
        return false;

    return whiteOpen || blackOpen;
}

constexpr auto Position::get_half_open_files() const noexcept
{
    return magic_enum::enum_values<File>()
         | std::views::filter([this](const File file) { return is_file_half_open(file); });
}

} // namespace chess::game
