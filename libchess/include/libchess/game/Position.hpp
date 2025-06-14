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

#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/game/ThreefoldChecker.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>

/** This namespace contains classes for modeling the state
    of a game of chess.

    @ingroup game
 */
namespace chess::game {

using board::Bitboard;
using board::File;
using board::Pieces;
using board::Square;
using moves::Move;
using pieces::Color;

using PieceType = pieces::Type;

/** This class models an instant in a game of chess.

    It describes where all the pieces are placed, as well as some
    additional metadata such as castling rights, side to move,
    en passant possibility, etc.

    @ingroup game

    @todo Funcs to get passed pawns, backward pawns
 */
struct Position final {
    /** Creates a Position object representing the starting position. */
    Position();

    /** The positions of the White pieces.

        @invariant The bitboard indices of the bits set in ``whitePieces``
        must not overlap with any of the indices of the bits set in
        ``blackPieces``.
     */
    Pieces whitePieces { Color::White };

    /** The positions of the Black pieces.

        @invariant The bitboard indices of the bits set in ``blackPieces``
        must not overlap with any of the indices of the bits set in
        ``whitePieces``.
     */
    Pieces blackPieces { Color::Black };

    /** Indicates whose move it is in this position. */
    Color sideToMove { Color::White };

    /** Castling rights for the White player. */
    CastlingRights whiteCastlingRights;

    /** Castling rights for the Black player. */
    CastlingRights blackCastlingRights;

    /** If the last move was a pawn double-push, then this holds
        the square that a pawn would land on after capturing
        en passant. This is always set to a non-null value if
        the last move was a pawn double-push, even if no enemy
        pawns are actually in position to perform the capture.
     */
    std::optional<Square> enPassantTargetSquare;

    /** This is a ply counter that enforces the 50-move rule.
        The counter is incremented after every move and reset by
        captures and pawn moves; if the counter reaches 100 and
        the side to move has at least 1 legal move, then the game
        is drawn.

        @invariant This value will never be greater than 100.

        @see is_fifty_move_draw()
     */
    std::uint_least8_t halfmoveClock { 0 };

    /** The number of full moves in the game.
        This value is monotonically increasing throughout the game,
        and is incremented after every Black move.
     */
    std::uint_least64_t fullMoveCounter { 1 };

    /** The Zobrist hash value of this position.
        This value is incrementally updated by the ``make_move()``
        function. If you manually change attributes of the position,
        call the ``refresh_zobrist()`` function to recalculate it.
     */
    std::uint64_t hash;

    /** Returns true if the two positions have the same Zobrist hash. */
    [[nodiscard]] bool operator==(const Position& other) const noexcept
    {
        return hash == other.hash;
    }

    /// @name Piece access
    /// @{

    /** Returns the piece set representing the given color. */
    template <Color Side>
    [[nodiscard]] Pieces& pieces_for() noexcept
    {
        if constexpr (Side == Color::White)
            return whitePieces;
        else
            return blackPieces;
    }

    /** Returns the piece set representing the given color. */
    template <Color Side>
    [[nodiscard]] const Pieces& pieces_for() const noexcept
    {
        if constexpr (Side == Color::White)
            return whitePieces;
        else
            return blackPieces;
    }

    /** Returns the pieces belonging to the side to move. */
    [[nodiscard]] Pieces& our_pieces() noexcept
    {
        if (sideToMove == Color::White)
            return whitePieces;

        return blackPieces;
    }

    /** Returns the pieces belonging to the side to move. */
    [[nodiscard]] const Pieces& our_pieces() const noexcept
    {
        if (sideToMove == Color::White)
            return whitePieces;

        return blackPieces;
    }

    /** Returns the pieces belonging to the side-to-move's opponent. */
    [[nodiscard]] Pieces& their_pieces() noexcept
    {
        if (sideToMove == Color::White)
            return blackPieces;

        return whitePieces;
    }

    /** Returns the pieces belonging to the side-to-move's opponent. */
    [[nodiscard]] const Pieces& their_pieces() const noexcept
    {
        if (sideToMove == Color::White)
            return blackPieces;

        return whitePieces;
    }

    /// @}

    /** Returns a bitboard that is the union of all White and Black
        piece positions.
     */
    [[nodiscard]] Bitboard occupied() const noexcept { return whitePieces.occupied | blackPieces.occupied; }

    /** Returns a bitboard that is the inverse of the ``occupied()`` board. */
    [[nodiscard]] Bitboard free() const noexcept { return occupied().inverse(); }

    /// @name File queries
    /// @{

    /** Returns true if there are no pawns of either color on the given file.
        @see get_open_files()
     */
    [[nodiscard]] bool is_file_open(File file) const noexcept;

    /** Returns an iterable range of File enumeration values corresponding
        to all open files in this position.

        @see is_file_open()
     */
    [[nodiscard]] auto get_open_files() const noexcept;

    /** Returns true if only one side has a pawn on the given file.
        @see get_half_open_files
     */
    [[nodiscard]] bool is_file_half_open(File file) const noexcept;

    /** Returns an iterable range of File enumeration values corresponding
        to all half-open files in this position.

        @see is_file_half_open()
     */
    [[nodiscard]] auto get_half_open_files() const noexcept;

    /// @}

    /** Returns true if the king of the side to move is in check. */
    [[nodiscard]] bool is_check() const noexcept { return is_side_in_check(sideToMove); }

    /// @name Game result queries
    /// @{

    /** Returns true if the king is attacked and the side to move has no legal moves. */
    [[nodiscard]] bool is_checkmate() const;

    /** Returns true if the side to move has no legal moves, but their king is not attacked. */
    [[nodiscard]] bool is_stalemate() const;

    /** Returns true if this position is a fifty-move draw, based on the ``halfmoveClock``.
        Note that in order for the game to be drawn, the side to move must have at least 1 legal move.
     */
    [[nodiscard]] bool is_fifty_move_draw() const;

    [[nodiscard]] bool is_threefold_repetition() const noexcept;

    /** Returns true if the game has concluded in a draw. */
    [[nodiscard]] bool is_draw() const;

    /** If this position is a conclusive draw or checkmate, returns the appropriate Result enumeration.
        Returns ``nullopt`` if this position is not a conclusive one.
     */
    [[nodiscard]] std::optional<Result> get_result() const;

    /// @}

    /** Returns true if the given move is legal (that is, the king is not left in check).
        This function does not verify piece movement mechanics or that a piece of the
        given type exists on the starting square; this function only verifies that making
        the move does not leave the side's king in check.
     */
    [[nodiscard]] bool is_legal(const Move& move) const;

    /** Returns true if the given move is an en passant capture in the current position. */
    [[nodiscard]] bool is_en_passant(const Move& move) const noexcept;

    /** Returns true if the given move is a capture, including en passant. */
    [[nodiscard]] bool is_capture(const Move& move) const noexcept;

    /** Makes a move to alter the position. */
    void make_move(const Move& move);

    /** Recalculates the Zobrist hash for this position. */
    void refresh_zobrist();

    /** Returns an empty position with none of the piece bitboards initialized.
        This is useful for tasks like parsing a FEN string, for example.
        After you've set up the position, don't forget to call ``whitePieces.refresh_occupied()``,
        ``blackPieces.refresh_occupied()``, and ``refresh_zobrist()`` to update
        all relevant cached state.
     */
    [[nodiscard]] static constexpr Position empty();

private:
    [[nodiscard]] bool is_side_in_check(Color side) const noexcept;

    ThreefoldChecker threefoldChecker { hash };
};

/** Returns a copy of the starting position with the given move applied.

    @note This function does not verify that the given move is legal.

    @relates Position
    @ingroup game
 */
[[nodiscard, gnu::const]] Position after_move(const Position& starting, const Move& move);

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

constexpr Position Position::empty()
{
    Position pos;

    pos.whitePieces = {};
    pos.blackPieces = {};

    return pos;
}

inline bool Position::is_threefold_repetition() const noexcept
{
    if (halfmoveClock < 8uz)
        return false;

    return threefoldChecker.is_threefold();
}

inline bool Position::is_side_in_check(const Color side) const noexcept
{
    if (side == Color::White) {
        return moves::squares_attacked<Color::Black>(
            blackPieces, whitePieces.king, whitePieces.occupied);
    }

    return moves::squares_attacked<Color::White>(
        whitePieces, blackPieces.king, blackPieces.occupied);
}

inline bool Position::is_legal(const Move& move) const
{
    auto copy { *this };

    copy.make_move(move);

    return ! copy.is_side_in_check(sideToMove);
}

inline bool Position::is_en_passant(const Move& move) const noexcept
{
    return move.piece == PieceType::Pawn
        && enPassantTargetSquare.has_value()
        && move.to == *enPassantTargetSquare;
}

inline bool Position::is_capture(const Move& move) const noexcept
{
    return is_en_passant(move) || their_pieces().occupied.test(move.to);
}

inline bool Position::is_file_open(const File file) const noexcept
{
    return whitePieces.is_file_half_open(file) && blackPieces.is_file_half_open(file);
}

inline auto Position::get_open_files() const noexcept
{
    return magic_enum::enum_values<File>()
         | std::views::filter([this](const File file) { return is_file_open(file); });
}

inline bool Position::is_file_half_open(const File file) const noexcept
{
    const bool whiteOpen = whitePieces.is_file_half_open(file);
    const bool blackOpen = blackPieces.is_file_half_open(file);

    // boolean XOR
    return whiteOpen != blackOpen;
}

inline auto Position::get_half_open_files() const noexcept
{
    return magic_enum::enum_values<File>()
         | std::views::filter([this](const File file) { return is_file_half_open(file); });
}

inline Position after_move(const Position& starting, const Move& move)
{
    auto copy { starting };

    copy.make_move(move);

    return copy;
}

} // namespace chess::game
