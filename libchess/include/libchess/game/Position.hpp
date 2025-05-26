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
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

/** This namespace contains classes for modeling the state
    of a game of chess.

    @ingroup game
 */
namespace chess::game {

using board::File;
using board::Rank;
using board::Square;
using moves::Move;
using pieces::Color;

using PieceType = pieces::Type;

/** This class models an instant in a game of chess.

    It describes where all the pieces are placed, as well as some
    additional metadata such as castling rights, side to move,
    en passant possibility, etc.

    @ingroup game

    @todo unmake_move()

    @todo Detect threefold reps by keeping array<Position, 6> ?

    @todo Funcs to get passed pawns, backward pawns
    @todo Funcs is_stalemate(), is_checkmate(), is_check()

    @todo std::hash
 */
struct Position final {
    /** The positions of the White pieces.

        @invariant The bitboard indices of the bits set in ``whitePieces``
        must not overlap with any of the indices of the bits set in
        ``blackPieces``.
     */
    board::Pieces whitePieces { Color::White };

    /** The positions of the Black pieces.

        @invariant The bitboard indices of the bits set in ``blackPieces``
        must not overlap with any of the indices of the bits set in
        ``whitePieces``.
     */
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
    std::optional<Square> enPassantTargetSquare;

    /** This is a ply counter that enforces the 50-move rule.
        The counter is incremented after every move and reset by
        captures and pawn moves; if the counter reaches 100 and
        the side to move has at least 1 legal move, then the game
        is drawn.

        @invariant This value will never be greater than 100.
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

    /** Makes a move to alter the position. */
    constexpr void make_move(const Move& move) noexcept;

    /** Converts a move to a string in algebraic notation.
        The current state of the position is used to determine whether the move is
        a capture, and the pieces of the side to move are used for disambiguation
        if necessary.
     */
    [[nodiscard]] std::string move_to_string(const Move& move) const;

    /** Creates a move from a string in algebraic notation, such as "Nd4", "e8=Q",
        "O-O-O", etc.

        @throws std::invalid_argument An exception will be thrown if a move cannot be
        parsed correctly from the input string.
     */
    [[nodiscard]] Move move_from_string(std::string_view text) const;
};

/** Creates a UTF8 representation of the given position.
    The returned string is meant to be interpreted visually by a human, probably for debugging purposes.
    The board is drawn as a simple set of cells separated by ``|`` characters. Pieces are drawn using
    their UTF8-encoded symbols.

    @relates Position
    @ingroup game
 */
[[nodiscard]] std::string print_utf8(const Position& position);

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

constexpr void Position::make_move(const Move& move) noexcept
{
    const bool isWhite = sideToMove == Color::White;

    const bool isPawnMove = move.piece == PieceType::Pawn;

    auto& opponentPieces = isWhite ? blackPieces : whitePieces;

    const bool isCapture = opponentPieces.occupied().test(move.to);

    // update bitboards
    {
        opponentPieces.capture_at(move.to);

        auto& ourPieces = isWhite ? whitePieces : blackPieces;
        auto& bitboard  = ourPieces.get_type(move.piece);

        bitboard.unset(move.from);

        if (move.is_promotion())
            ourPieces.get_type(*move.promotedType).set(move.to);
        else
            bitboard.set(move.to);

        if (move.is_castling()) { // update rook position
            const bool castledQueenside = move.to.is_queenside();

            auto& rooks = ourPieces.rooks;

            if (castledQueenside) {
                rooks.unset(Square { File::A, move.to.rank });
                rooks.set(Square { File::D, move.to.rank });
            } else {
                rooks.unset(Square { File::H, move.to.rank });
                rooks.set(Square { File::F, move.to.rank });
            }
        }
    }

    enPassantTargetSquare = [move, isWhite, isPawnMove]() -> std::optional<Square> {
        if (! isPawnMove)
            return std::nullopt;

        if (std::cmp_equal(board::rank_distance(move.from, move.to), 2uz))
            return Square {
                .file = move.to.file,
                .rank = isWhite ? Rank::Three : Rank::Six
            };

        return std::nullopt;
    }();

    // update castling rights
    auto& castlingRights = isWhite ? whiteCastlingRights : blackCastlingRights;

    if (move.piece == PieceType::King)
        castlingRights.king_moved();
    else if (move.piece == PieceType::Rook)
        castlingRights.rook_moved(move.from.is_kingside());

    // update halfmoveClock
    if (isPawnMove || isCapture)
        halfmoveClock = 0;
    else if (std::cmp_greater_equal(halfmoveClock, 100))
        ; // TODO: game ends in draw
    else
        ++halfmoveClock;

    // flip side to move
    sideToMove = isWhite ? Color::Black : Color::White;
}

} // namespace chess::game
