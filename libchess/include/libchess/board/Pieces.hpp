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
    This file defines the Pieces class.
    @ingroup board
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <utility> // IWYU pragma: keep - for std::unreachable()

namespace chess::board {

using pieces::Color;

using PieceType = pieces::Type;

/** This class encapsulates a bitboard for each piece type.
    An instance of this class encodes the locations of all pieces for one side.

    @invariant No more than one piece type may occupy a given square; that is, each
    bitboard index will only have its bit set in at most one of the piece type bitboards.

    @ingroup board
 */
struct Pieces final {
    /** Creates a Pieces object encoding the starting position for the given side. */
    explicit constexpr Pieces(Color color) noexcept;

    /** Creates an empty Pieces object with none of its bitboards initialized. */
    consteval Pieces() noexcept = default;

    /** This bitboard holds the locations of this side's pawns. */
    Bitboard pawns;

    /** This bitboard holds the locations of this side's knights. */
    Bitboard knights;

    /** This bitboard holds the locations of this side's bishops. */
    Bitboard bishops;

    /** This bitboard holds the locations of this side's rooks. */
    Bitboard rooks;

    /** This bitboard holds the locations of this side's queens. */
    Bitboard queens;

    /** This bitboard holds the locations of this side's king.

        @invariant This board will never have more than a single bit set.
     */
    Bitboard king;

    /** This bitboard is a composite of all of this side's pieces.
        This board is kept up to date by the capture_at() and our_move() methods.
        If you update the individual piece boards manually, you must also update
        this board, or call refresh_occupied() to flush and repopulate it.
     */
    Bitboard occupied {
        pawns | knights | bishops | rooks | queens | king
    };

    /** Returns true if the two piece sets are identical. */
    [[nodiscard]] constexpr auto operator==(const Pieces&) const noexcept -> bool = default;

    /** Returns the bitboard corresponding to the given piece type. */
    [[nodiscard]] constexpr auto get_type(PieceType type) noexcept -> Bitboard&;

    /** Returns the bitboard corresponding to the given piece type. */
    [[nodiscard]] constexpr auto get_type(PieceType type) const noexcept -> Bitboard;

    /** Returns a bitboard that is the inverse of the bitboard returned by ``occupied()``.
        The returned bitboard has a bit set if no piece of any type is on that square.
     */
    [[nodiscard]] constexpr auto free() const noexcept -> Bitboard { return occupied.inverse(); }

    /** Returns true if there are no pawns of this color anywhere on the given file. */
    [[nodiscard]] constexpr auto is_file_half_open(File file) const noexcept -> bool;

    /** Returns true if this side has at least one bishop on each color complex. */
    [[nodiscard]] constexpr auto has_bishop_pair() const noexcept -> bool;

    /** Returns the location of the king. */
    [[nodiscard]] constexpr auto get_king_location() const noexcept -> Square;

    /** Returns the type of the piece on the given square, or ``nullopt`` if the
        square is empty. Note that libchess's bitboard board representation is
        not optimized for this operation.
     */
    [[nodiscard]] constexpr auto get_piece_on(Square square) const noexcept
        -> std::optional<PieceType>;

    /** Removes the piece on the given square, if any.
        This method asserts if the ``square`` is the location of the king.
     */
    constexpr void capture_at(Square square) noexcept;

    /** Call this when a move is made by this side to update the piece bitboards. */
    constexpr void our_move(moves::Move move, Color ourColor) noexcept;

    /** Recalculates the ``occupied`` bitboard from each of the piece bitboards. */
    constexpr void refresh_occupied() noexcept;

    /** Performs a vertical flip of all piece bitboards and refreshes the occupied bitboard. */
    constexpr void vertical_flip() noexcept;
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

constexpr Pieces::Pieces(const Color color) noexcept
    : pawns { masks::starting::pawns(color) }
    , knights { masks::starting::knights(color) }
    , bishops { masks::starting::bishops(color) }
    , rooks { masks::starting::rooks(color) }
    , queens { masks::starting::queen(color) }
    , king { masks::starting::king(color) }
{
}

constexpr void Pieces::refresh_occupied() noexcept
{
    occupied = pawns | knights | bishops | rooks | queens | king;
}

constexpr void Pieces::vertical_flip() noexcept
{
    using flips::vertical;

    pawns   = vertical(pawns);
    knights = vertical(knights);
    bishops = vertical(bishops);
    rooks   = vertical(rooks);
    queens  = vertical(queens);
    king    = vertical(king);

    refresh_occupied();
}

constexpr auto Pieces::get_type(const PieceType type) noexcept -> Bitboard&
{
    switch (type) {
        case PieceType::Knight: return knights;
        case PieceType::Bishop: return bishops;
        case PieceType::Rook  : return rooks;
        case PieceType::Queen : return queens;
        case PieceType::King  : return king;
        case PieceType::Pawn  : return pawns;
        default               : std::unreachable();
    }
}

constexpr auto Pieces::get_type(const PieceType type) const noexcept -> Bitboard
{
    switch (type) {
        case PieceType::Knight: return knights;
        case PieceType::Bishop: return bishops;
        case PieceType::Rook  : return rooks;
        case PieceType::Queen : return queens;
        case PieceType::King  : return king;
        case PieceType::Pawn  : return pawns;
        default               : std::unreachable();
    }
}

constexpr auto Pieces::is_file_half_open(const File file) const noexcept -> bool
{
    return (pawns & masks::files::get(file)).none();
}

constexpr auto Pieces::has_bishop_pair() const noexcept -> bool
{
    // NB. can't just check that bishops.count() == 2 because
    // it's possible that we have 2 bishops of the same color

    return (bishops & masks::LIGHT_SQUARES).any()
       and (bishops & masks::DARK_SQUARES).any();
}

constexpr auto Pieces::get_king_location() const noexcept -> Square
{
    assert(std::cmp_equal(king.count(), 1));

    return Square::from_index(king.first());
}

constexpr auto Pieces::get_piece_on(const Square square) const noexcept
    -> std::optional<PieceType>
{
    if (not occupied.test(square))
        return std::nullopt;

    static constexpr auto allTypes = magic_enum::enum_values<PieceType>();

    if (const auto found = std::ranges::find_if(
            allTypes,
            [this, index = square.index()](const PieceType type) {
                return get_type(type).test(index);
            });
        found != allTypes.end()) {
        // we should find a piece unless the occupied bitboard has gotten out of sync
        [[likely]];

        return *found;
    }

    assert(false);

    return std::nullopt;
}

constexpr void Pieces::capture_at(const Square square) noexcept
{
    const auto idx = square.index();

    // if we're trying to capture the king, then an illegal move has already been played
    assert(not king.test(idx));

    pawns.unset(idx);
    knights.unset(idx);
    bishops.unset(idx);
    rooks.unset(idx);
    queens.unset(idx);

    occupied.unset(idx);
}

constexpr void Pieces::our_move(const moves::Move move, const Color ourColor) noexcept
{
    const auto movementMask = Bitboard::from_square(move.from()) | Bitboard::from_square(move.to());

    occupied ^= movementMask;

    move.promoted_type()
        .transform([move, this](const PieceType type) {
            get_type(move.piece()).unset(move.from());
            get_type(type).set(move.to());
            return std::monostate {};
        })
        .or_else([move, movementMask, ourColor, this] {
            get_type(move.piece()) ^= movementMask;

            if (move.is_castling()) {
                const auto castleMask = move.to().is_queenside()
                                          ? masks::queenside_castle_rook_pos_mask(ourColor)
                                          : masks::kingside_castle_rook_pos_mask(ourColor);

                rooks ^= castleMask;
                occupied ^= castleMask;
            }

            return std::make_optional(std::monostate {});
        });
}

} // namespace chess::board
