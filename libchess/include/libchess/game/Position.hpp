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

#include <cassert>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/CastlingRights.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/game/ThreefoldChecker.hpp>
#include <libchess/game/Zobrist.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <utility>

/** This namespace contains classes for modeling the state
    of a game of chess.

    @ingroup game
 */
namespace chess::game {

using board::Bitboard;
using board::File;
using board::Pieces;
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

    @todo Funcs to get passed pawns, backward pawns
 */
struct Position final {
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
    zobrist::Value hash {
        zobrist::calculate(sideToMove,
            whitePieces, blackPieces,
            whiteCastlingRights, blackCastlingRights,
            enPassantTargetSquare)
    };

    /** Returns true if the two positions have the same Zobrist hash. */
    [[nodiscard]] constexpr bool operator==(const Position& other) const noexcept
    {
        return hash == other.hash;
    }

    /// @name Piece access
    /// @{

    /** Returns the piece set representing the given color. */
    template <Color Side>
    [[nodiscard]] constexpr Pieces& pieces_for() noexcept
    {
        if constexpr (Side == Color::White)
            return whitePieces;
        else
            return blackPieces;
    }

    /** Returns the piece set representing the given color. */
    template <Color Side>
    [[nodiscard]] constexpr const Pieces& pieces_for() const noexcept
    {
        if constexpr (Side == Color::White)
            return whitePieces;
        else
            return blackPieces;
    }

    /** Returns the pieces belonging to the side to move. */
    [[nodiscard]] constexpr Pieces& our_pieces() noexcept
    {
        if (sideToMove == Color::White)
            return whitePieces;

        return blackPieces;
    }

    /** Returns the pieces belonging to the side to move. */
    [[nodiscard]] constexpr const Pieces& our_pieces() const noexcept
    {
        if (sideToMove == Color::White)
            return whitePieces;

        return blackPieces;
    }

    /** Returns the pieces belonging to the side-to-move's opponent. */
    [[nodiscard]] constexpr Pieces& their_pieces() noexcept
    {
        if (sideToMove == Color::White)
            return blackPieces;

        return whitePieces;
    }

    /** Returns the pieces belonging to the side-to-move's opponent. */
    [[nodiscard]] constexpr const Pieces& their_pieces() const noexcept
    {
        if (sideToMove == Color::White)
            return blackPieces;

        return whitePieces;
    }

    /// @}

    /** Returns a bitboard that is the union of all White and Black
        piece positions.
     */
    [[nodiscard]] constexpr Bitboard occupied() const noexcept { return whitePieces.occupied | blackPieces.occupied; }

    /** Returns a bitboard that is the inverse of the ``occupied()`` board. */
    [[nodiscard]] constexpr Bitboard free() const noexcept { return occupied().inverse(); }

    /// @name File queries
    /// @{

    /** Returns true if there are no pawns of either color on the given file.
        @see get_open_files()
     */
    [[nodiscard]] constexpr bool is_file_open(File file) const noexcept;

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

    /// @}

    /** Returns true if the king of the side to move is in check. */
    [[nodiscard]] constexpr bool is_check() const noexcept;

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

    [[nodiscard]] constexpr bool is_threefold_repetition() const noexcept;

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
    [[nodiscard]] constexpr bool is_legal(const Move& move) const;

    /** Returns true if the given move is an en passant capture in the current position. */
    [[nodiscard]] constexpr bool is_en_passant(const Move& move) const noexcept;

    /** Returns true if the given move is a capture, including en passant. */
    [[nodiscard]] constexpr bool is_capture(const Move& move) const noexcept;

    /** Makes a move to alter the position. */
    constexpr void make_move(const Move& move);

    /** Recalculates the Zobrist hash for this position. */
    constexpr void refresh_zobrist();

    /** Returns an empty position with none of the piece bitboards initialized.
        This is useful for tasks like parsing a FEN string, for example.
        After you've set up the position, don't forget to call ``whitePieces.refresh_occupied()``,
        ``blackPieces.refresh_occupied()``, and ``refresh_zobrist()`` to update
        all relevant cached state.
     */
    [[nodiscard]] static constexpr Position empty();

private:
    [[nodiscard]] constexpr bool is_side_in_check(Color side) const noexcept;

    ThreefoldChecker threefoldChecker { hash };
};

/** Returns a copy of the starting position with the given move applied.

    @note This function does not verify that the given move is legal.

    @relates Position
    @ingroup game
 */
[[nodiscard, gnu::const]] constexpr Position after_move(const Position& starting, const Move& move);

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

constexpr bool Position::is_threefold_repetition() const noexcept
{
    if (halfmoveClock < 8uz)
        return false;

    return threefoldChecker.is_threefold();
}

constexpr bool Position::is_check() const noexcept
{
    return is_side_in_check(sideToMove);
}

constexpr bool Position::is_side_in_check(const Color side) const noexcept
{
    if (side == Color::White) {
        return squares_attacked<Color::Black>(
            blackPieces, whitePieces.king, whitePieces.occupied);
    }

    return squares_attacked<Color::White>(
        whitePieces, blackPieces.king, blackPieces.occupied);
}

constexpr bool Position::is_legal(const Move& move) const
{
    auto copy { *this };

    copy.make_move(move);

    return ! copy.is_side_in_check(sideToMove);
}

constexpr bool Position::is_en_passant(const Move& move) const noexcept
{
    return move.piece == PieceType::Pawn
        && enPassantTargetSquare.has_value()
        && move.to == *enPassantTargetSquare;
}

constexpr bool Position::is_capture(const Move& move) const noexcept
{
    return is_en_passant(move) || their_pieces().occupied.test(move.to);
}

constexpr bool Position::is_file_open(const File file) const noexcept
{
    return whitePieces.is_file_half_open(file) && blackPieces.is_file_half_open(file);
}

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
    return whiteOpen != blackOpen;
}

constexpr auto Position::get_half_open_files() const noexcept
{
    return magic_enum::enum_values<File>()
         | std::views::filter([this](const File file) { return is_file_half_open(file); });
}

constexpr void Position::refresh_zobrist()
{
    hash = zobrist::calculate(sideToMove,
        whitePieces, blackPieces,
        whiteCastlingRights, blackCastlingRights,
        enPassantTargetSquare);

    threefoldChecker.reset(hash);
}

namespace detail {

    // given the en passant target square, this returns the square that the
    // captured pawn was on
    [[nodiscard, gnu::const]] constexpr Square get_en_passant_captured_square(
        const Square& targetSquare, const bool isWhite) noexcept
    {
        // the captured pawn is on the file of the target square, but one file below
        // (White capture) or one file above (Black capture)
        const auto capturedRank = isWhite
                                    ? board::prev_pawn_rank<Color::White>(targetSquare.rank)
                                    : board::prev_pawn_rank<Color::Black>(targetSquare.rank);

        return Square {
            .file = targetSquare.file,
            .rank = capturedRank
        };
    }

    constexpr void update_bitboards(
        Position& position, const Move& move) noexcept
    {
        const bool isWhite = position.sideToMove == Color::White;

        auto& ourPieces      = position.our_pieces();
        auto& opponentPieces = position.their_pieces();

        opponentPieces.capture_at(move.to);

        ourPieces.our_move(move, position.sideToMove);

        if (position.is_en_passant(move)) {
            [[unlikely]];

            const auto idx = get_en_passant_captured_square(
                position.enPassantTargetSquare.value(), isWhite)
                                 .index();

            opponentPieces.pawns.unset(idx);
            opponentPieces.occupied.unset(idx);
        }
    }

    [[nodiscard, gnu::const]] constexpr std::optional<Square> get_en_passant_target_square(
        const Move& move, const bool isWhite) noexcept
    {
        if (move.piece != PieceType::Pawn
            || std::cmp_not_equal(board::rank_distance(move.from, move.to), 2uz)) {
            [[likely]];
            return std::nullopt;
        }

        return Square {
            .file = move.to.file,
            .rank = isWhite ? Rank::Three : Rank::Six
        };
    }

    // each of these bools are true if the given right has changed since the last move
    struct CastlingRightsChanges final {
        bool whiteKingside { false };
        bool whiteQueenside { false };
        bool blackKingside { false };
        bool blackQueenside { false };
    };

    [[nodiscard]] constexpr CastlingRightsChanges update_castling_rights(
        Position& pos, const bool isWhite, const Move& move) noexcept
    {
        const auto whiteOldRights { pos.whiteCastlingRights };
        const auto blackOldRights { pos.blackCastlingRights };

        auto& ourRights   = isWhite ? pos.whiteCastlingRights : pos.blackCastlingRights;
        auto& theirRights = isWhite ? pos.blackCastlingRights : pos.whiteCastlingRights;

        ourRights.our_move(move);

        if (isWhite)
            theirRights.their_move<Color::Black>(move);
        else
            theirRights.their_move<Color::White>(move);

        const auto& whiteNewRights = pos.whiteCastlingRights;
        const auto& blackNewRights = pos.blackCastlingRights;

        return {
            .whiteKingside  = whiteOldRights.kingside != whiteNewRights.kingside,
            .whiteQueenside = whiteOldRights.queenside != whiteNewRights.queenside,
            .blackKingside  = blackOldRights.kingside != blackNewRights.kingside,
            .blackQueenside = blackOldRights.queenside != blackNewRights.queenside
        };
    }

    [[nodiscard, gnu::const]] constexpr std::uint_least8_t tick_halfmove_clock(
        const Move& move, const bool isCapture, const std::uint_least8_t prevValue) noexcept
    {
        if (isCapture || (move.piece == PieceType::Pawn))
            return 0;

        static constexpr auto MAX_VALUE = static_cast<std::uint_least8_t>(100);

        if (std::cmp_greater_equal(prevValue, MAX_VALUE)) {
            [[unlikely]];
            return MAX_VALUE;
        }

        return prevValue + 1;
    }

    [[nodiscard, gnu::const]] constexpr zobrist::Value update_zobrist(
        const Position& pos, const Move& move,
        const std::optional<Square>  newEPTarget,
        const CastlingRightsChanges& rightsChanges)
    {
        auto value = pos.hash;

        value ^= zobrist::BLACK_TO_MOVE; // just toggle these bits in/out every other move

        if (pos.enPassantTargetSquare.has_value())
            value ^= zobrist::en_passant_key(pos.enPassantTargetSquare->file);

        if (newEPTarget.has_value())
            value ^= zobrist::en_passant_key(newEPTarget->file);

        value ^= zobrist::piece_key(move.piece, pos.sideToMove, move.from);

        value ^= zobrist::piece_key(
            move.is_promotion() ? *move.promotedType : move.piece,
            pos.sideToMove, move.to);

        if (pos.is_capture(move)) {
            const auto otherColor = pos.sideToMove == Color::White ? Color::Black : Color::White;

            if (pos.is_en_passant(move)) {
                [[unlikely]];

                value ^= zobrist::piece_key(
                    PieceType::Pawn, otherColor,
                    get_en_passant_captured_square(
                        pos.enPassantTargetSquare.value(),
                        pos.sideToMove == Color::White));
            } else {
                [[likely]];

                const auto capturedType = pos.their_pieces().get_piece_on(move.to);

                assert(capturedType.has_value());

                value ^= zobrist::piece_key(*capturedType, otherColor, move.to);
            }
        } else if (move.is_castling()) {
            [[unlikely]];
            if (move.to.is_kingside())
                value ^= board::masks::kingside_castle_rook_pos_mask(pos.sideToMove).to_int();
            else
                value ^= board::masks::queenside_castle_rook_pos_mask(pos.sideToMove).to_int();
        }

        if (rightsChanges.whiteKingside)
            value ^= zobrist::WHITE_KINGSIDE_CASTLE;

        if (rightsChanges.whiteQueenside)
            value ^= zobrist::WHITE_QUEENSIDE_CASTLE;

        if (rightsChanges.blackKingside)
            value ^= zobrist::BLACK_KINGSIDE_CASTLE;

        if (rightsChanges.blackQueenside)
            value ^= zobrist::BLACK_QUEENSIDE_CASTLE;

        return value;
    }

} // namespace detail

constexpr void Position::make_move(const Move& move)
{
    const bool isCapture = is_capture(move);
    const bool isWhite   = sideToMove == Color::White;

    const auto newEPSquare = detail::get_en_passant_target_square(move, isWhite);

    const auto rightsChanges = detail::update_castling_rights(*this, isWhite, move);

    hash = detail::update_zobrist(*this, move, newEPSquare, rightsChanges);

    detail::update_bitboards(*this, move);

    halfmoveClock = detail::tick_halfmove_clock(move, isCapture, halfmoveClock);

    enPassantTargetSquare = newEPSquare;

    // increment full move counter after every Black move
    if (! isWhite)
        ++fullMoveCounter;

    // flip side to move
    sideToMove = isWhite ? Color::Black : Color::White;

    threefoldChecker.push(hash);
}

constexpr Position after_move(const Position& starting, const Move& move)
{
    auto copy { starting };

    copy.make_move(move);

    return copy;
}

} // namespace chess::game
