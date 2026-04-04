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

/** @defgroup game Modeling of the chess game
    Classes to model the state of a game of chess.

    @ingroup libchess
 */

/** @file
    This file defines the Position class.
    @ingroup game
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/game/ThreefoldChecker.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>

namespace chess::moves {
struct Move;
} // namespace chess::moves

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
 */
struct [[nodiscard]] Position final {
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
    std::uint_least8_t halfmoveClock { UINT8_C(0) };

    /** The number of full moves in the game.
        This value is monotonically increasing throughout the game,
        and is incremented after every Black move.
     */
    std::uint_least64_t fullMoveCounter { UINT64_C(1) };

    /** Unsigned 64-bit integer used for hash values. */
    using Hash = std::uint64_t;

    /** The Zobrist hash value of this position.
        This value is incrementally updated by the ``make_move()``
        function. If you manually change attributes of the position,
        call the ``refresh_zobrist()`` function to recalculate it.
     */
    Hash hash { UINT64_C(0) };

    /** Returns true if the two positions have the same Zobrist hash. */
    [[nodiscard]] auto operator==(const Position& other) const noexcept -> bool
    {
        return hash == other.hash;
    }

    /// @name Piece access
    /// @{

    /** Returns the piece set representing the given color. */
    template <Color Side>
    [[nodiscard]] auto pieces_for() noexcept -> Pieces&
    {
        if constexpr (Side == Color::White)
            return whitePieces;
        else
            return blackPieces;
    }

    /** Returns the piece set representing the given color. */
    template <Color Side>
    [[nodiscard]] auto pieces_for() const noexcept -> const Pieces&
    {
        if constexpr (Side == Color::White)
            return whitePieces;
        else
            return blackPieces;
    }

    /** Returns the castling rights for the given color. */
    template <Color Side>
    [[nodiscard]] auto castling_rights_for() const noexcept -> CastlingRights
    {
        if constexpr (Side == Color::White)
            return whiteCastlingRights;
        else
            return blackCastlingRights;
    }

    /** Returns the pieces belonging to the side to move. */
    [[nodiscard]] auto our_pieces() noexcept -> Pieces&
    {
        if (is_white_to_move())
            return whitePieces;

        return blackPieces;
    }

    /** Returns the pieces belonging to the side to move. */
    [[nodiscard]] auto our_pieces() const noexcept -> const Pieces&
    {
        if (is_white_to_move())
            return whitePieces;

        return blackPieces;
    }

    /** Returns the pieces belonging to the side-to-move's opponent. */
    [[nodiscard]] auto their_pieces() noexcept -> Pieces&
    {
        if (is_white_to_move())
            return blackPieces;

        return whitePieces;
    }

    /** Returns the pieces belonging to the side-to-move's opponent. */
    [[nodiscard]] auto their_pieces() const noexcept -> const Pieces&
    {
        if (is_white_to_move())
            return blackPieces;

        return whitePieces;
    }

    /// @}

    /** Returns true if the side to move is White. */
    [[nodiscard]] auto is_white_to_move() const noexcept -> bool
    {
        return sideToMove == Color::White;
    }

    /** Returns true if the side to move is Black. */
    [[nodiscard]] auto is_black_to_move() const noexcept -> bool
    {
        return sideToMove == Color::Black;
    }

    /** Returns a bitboard that is the union of all White and Black
        piece positions.
     */
    [[nodiscard]] auto occupied() const noexcept -> Bitboard { return whitePieces.occupied | blackPieces.occupied; }

    /** Returns a bitboard that is the inverse of the ``occupied()`` board. */
    [[nodiscard]] auto free() const noexcept -> Bitboard { return occupied().inverse(); }

    /// @name File queries
    /// @{

    /** Returns true if there are no pawns of either color on the given file.
        @see get_open_files()
     */
    [[nodiscard]] auto is_file_open(File file) const noexcept -> bool;

    /** Returns an iterable range of File enumeration values corresponding
        to all open files in this position.

        @see is_file_open()
     */
    [[nodiscard]] auto get_open_files() const noexcept;

    /** Returns true if only one side has a pawn on the given file.
        @see get_half_open_files
     */
    [[nodiscard]] auto is_file_half_open(File file) const noexcept -> bool;

    /** Returns an iterable range of File enumeration values corresponding
        to all half-open files in this position.

        @see is_file_half_open()
     */
    [[nodiscard]] auto get_half_open_files() const noexcept;

    /// @}

    /** Returns true if the king of the side to move is in check. */
    [[nodiscard]] auto is_check() const noexcept -> bool { return is_side_in_check(sideToMove); }

    /** Returns a bitboard containing the locations of passed pawns for the given side. */
    template <Color Side>
    [[nodiscard]] auto get_passed_pawns() const noexcept -> Bitboard;

    /** Returns a bitboard containing the locations of backward pawns for the given side. */
    template <Color Side>
    [[nodiscard]] auto get_backward_pawns() const noexcept -> Bitboard;

    /// @name Game result queries
    /// @{

    /** Returns true if the king is attacked and the side to move has no legal moves. */
    [[nodiscard]] auto is_checkmate() const -> bool;

    /** Returns true if the side to move has no legal moves, but their king is not attacked. */
    [[nodiscard]] auto is_stalemate() const -> bool;

    /** Returns true if this position is a fifty-move draw, based on the ``halfmoveClock``.
        Note that in order for the game to be drawn, the side to move must have at least 1 legal move.
     */
    [[nodiscard]] auto is_fifty_move_draw() const -> bool;

    /** Returns true if this position is a draw by threefold repetition. */
    [[nodiscard]] auto is_threefold_repetition() const noexcept -> bool;

    /** Returns true if this position is either lone kings or lone king vs. king and bishop or knight. */
    [[nodiscard]] auto is_draw_by_insufficient_material() const noexcept -> bool;

    /** Returns true if the game has concluded in a draw. */
    [[nodiscard]] auto is_draw() const -> bool;

    /** If this position is a conclusive draw or checkmate, returns the appropriate Result enumeration.
        Returns ``nullopt`` if this position is not a conclusive one.
     */
    [[nodiscard]] auto get_result() const -> std::optional<Result>;

    /// @}

    /** Returns true if the given move is legal (that is, the king is not left in check).
        This function does not verify piece movement mechanics or that a piece of the
        given type exists on the starting square; this function only verifies that making
        the move does not leave the side's king in check.
     */
    [[nodiscard]] auto is_legal(Move move) const -> bool;

    /** Returns true if the given move is an en passant capture in the current position. */
    [[nodiscard]] auto is_en_passant(Move move) const noexcept -> bool;

    /** Returns true if the given move is a capture, including en passant. */
    [[nodiscard]] auto is_capture(Move move) const noexcept -> bool;

    /** Returns true if the given move is quiet - ie, does not alter material. */
    [[nodiscard]] auto is_quiet(Move move) const noexcept -> bool;

    /** Makes a move to alter the position. */
    void make_move(Move move);

    /** Makes a null move.
        This essentially represents the side to move "passing" their turn.
        This does update the Zobrist hash, halfmove clock and fullmove counter in the same
        way that ``make_move()`` does.
     */
    void make_null_move();

    /** Performs a color flip of this position. */
    void flip();

    /** Recalculates the Zobrist hash for this position. */
    void refresh_zobrist();

    /** Tests if the position is legal. If it is, returns ``nullopt``; if the position is
        illegal, returns an explanatory string describing the error condition detecting.
        This function conducts basic checks about the number of each type of piece, but this
        is not an exhaustive validation that a position can definitively be reached from
        the starting position.
     */
    [[nodiscard]] auto is_illegal() const -> std::optional<std::string>;

    /** If the en passant target square is set to a square that is not a potentially legal
        en passant target, this function resets the EP square to ``nullopt``.
     */
    void sanitize_ep_square();

    /** Returns an empty position with none of the piece bitboards initialized.
        This is useful for tasks like parsing a FEN string, for example.
        After you've set up the position, don't forget to call ``whitePieces.refresh_occupied()``,
        ``blackPieces.refresh_occupied()``, and ``refresh_zobrist()`` to update
        all relevant cached state.
     */
    [[nodiscard]] static auto empty() -> Position;

private:
    [[nodiscard]] auto is_side_in_check(Color side) const noexcept -> bool;

    ThreefoldChecker threefoldChecker;
};

/** Returns a copy of the starting position with the given move applied.

    @note This function does not verify that the given move is legal.

    @relates Position
    @ingroup game
    @see after_null_move()
 */
[[nodiscard, gnu::const]] auto after_move(
    const Position& starting, Move move)
    -> Position;

/** Returns a copy of the starting position with the side to move flipped.

    @relates Position
    @ingroup game
    @see after_move(), flipped()
 */
[[nodiscard, gnu::const]] auto after_null_move(const Position& starting) -> Position;

/** Returns a copy of the starting position with colors flipped.

    @relates Position
    @ingroup game
    @see after_null_move()
 */
[[nodiscard, gnu::const]] auto flipped(const Position& starting) -> Position;

/** Creates a UTF8 representation of the given position.
    The returned string is meant to be interpreted visually by a human, probably for debugging purposes.
    The board is drawn as a simple set of cells separated by ``|`` characters. Pieces are drawn using
    their UTF8-encoded symbols.

    @relates Position
    @ingroup game
    @see print_ascii()
 */
[[nodiscard]] auto print_utf8(const Position& position) -> std::string;

/** Creates an ASCII representation of the given position.
    The returned string is meant to be interpreted visually by a human, probably for debugging purposes.
    The board is drawn as a simple set of cells separated by ``|`` characters. Pieces are represented
    using letters (uppercase for white, lowercase for black).

    @relates Position
    @ingroup game
    @see print_utf8()
 */
[[nodiscard]] auto print_ascii(const Position& position) -> std::string;

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

inline auto Position::empty() -> Position
{
    Position pos { };

    pos.whitePieces = { };
    pos.blackPieces = { };

    pos.refresh_zobrist();

    return pos;
}

inline auto Position::is_threefold_repetition() const noexcept -> bool
{
    return threefoldChecker.is_threefold();
}

inline auto Position::is_side_in_check(const Color side) const noexcept -> bool
{
    if (side == Color::White) {
        return moves::squares_attacked<Color::Black>(
            blackPieces, whitePieces.king, whitePieces.occupied);
    }

    return moves::squares_attacked<Color::White>(
        whitePieces, blackPieces.king, blackPieces.occupied);
}

inline auto Position::is_legal(const Move move) const -> bool
{
    auto copy { *this };

    copy.make_move(move);

    return not copy.is_side_in_check(sideToMove);
}

inline auto Position::is_en_passant(const Move move) const noexcept -> bool
{
    return move.piece() == PieceType::Pawn
       and enPassantTargetSquare.has_value()
       and move.to() == *enPassantTargetSquare;
}

inline auto Position::is_capture(const Move move) const noexcept -> bool
{
    return is_en_passant(move) or their_pieces().occupied.test(move.to());
}

inline auto Position::is_quiet(const Move move) const noexcept -> bool
{
    return not(move.is_promotion() or is_capture(move));
}

inline auto Position::is_file_open(const File file) const noexcept -> bool
{
    return whitePieces.is_file_half_open(file) and blackPieces.is_file_half_open(file);
}

inline auto Position::get_open_files() const noexcept
{
    return magic_enum::enum_values<File>()
         | std::views::filter([this](const File file) { return is_file_open(file); });
}

inline auto Position::is_file_half_open(const File file) const noexcept -> bool
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

template <Color Side>
auto Position::get_passed_pawns() const noexcept -> Bitboard
{
    static constexpr auto OtherSide = pieces::other_side<Side>();

    const auto friendlyPawns = pieces_for<Side>().pawns;
    const auto enemyPawns    = pieces_for<OtherSide>().pawns;

    const auto mask = board::fills::pawn_front<OtherSide>(
        enemyPawns | moves::patterns::pawn_attacks<OtherSide>(enemyPawns));

    return friendlyPawns & mask.inverse();
}

template <Color Side>
auto Position::get_backward_pawns() const noexcept -> Bitboard
{
    static constexpr auto OtherSide = pieces::other_side<Side>();

    const auto friendlyPawns = pieces_for<Side>().pawns;
    const auto enemyPawns    = pieces_for<OtherSide>().pawns;

    const auto ourAttackSpans = board::fills::pawn_front<Side>(
        moves::patterns::pawn_attacks<Side>(friendlyPawns));

    const auto theirAttacks = moves::patterns::pawn_attacks<OtherSide>(enemyPawns);

    const auto backwardArea = board::fills::pawn_rear<Side>(
        ourAttackSpans.inverse() & theirAttacks);

    return backwardArea & friendlyPawns;
}

inline auto after_move(
    const Position& starting, const Move move)
    -> Position
{
    auto copy { starting };

    copy.make_move(move);

    return copy;
}

inline auto after_null_move(const Position& starting) -> Position
{
    auto copy { starting };

    copy.make_null_move();

    return copy;
}

inline auto flipped(const Position& starting) -> Position
{
    auto copy { starting };

    copy.flip();

    return copy;
}

} // namespace chess::game
