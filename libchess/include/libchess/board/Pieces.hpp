/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
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
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>

namespace chess::board {

using pieces::Color;
using std::size_t;

using PieceType = pieces::Type;

/** This class encapsulates a bitboard for each piece type.
    An instance of this class encodes the locations of all pieces for one side.

    @invariant No more than one piece type may occupy a given square; that is, each
    bitboard index will only have its bit set in at most one of the piece type bitboards.

    @ingroup board

    @todo func to check for doubled pawns
 */
struct Pieces final {
    /** Creates a Pieces object encoding the starting position for the given side. */
    explicit constexpr Pieces(Color color) noexcept;

    /** Creates an empty Pieces object with none of its bitboards initialized. */
    constexpr Pieces() noexcept = default;

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

    /** Returns true if the two piece sets are identical. */
    [[nodiscard]] constexpr bool operator==(const Pieces&) const noexcept = default;

    /** Returns the bitboard corresponding to the given piece type. */
    [[nodiscard]] constexpr Bitboard& get_type(PieceType type) noexcept;

    /** Returns the bitboard corresponding to the given piece type. */
    [[nodiscard]] constexpr Bitboard get_type(PieceType type) const noexcept;

    /** Returns a bitboard that is a union of each individual piece-type bitboard.
        The returned bitboard has a bit set if a piece of any type is on that square.
     */
    [[nodiscard]] constexpr Bitboard occupied() const noexcept;

    /** Returns a bitboard that is the inverse of the bitboard returned by ``occupied()``.
        The returned bitboard has a bit set if no piece of any type is on that square.
     */
    [[nodiscard]] constexpr Bitboard free() const noexcept { return occupied().inverse(); }

    /** Returns the sum of the material values for all pieces on this side. */
    [[nodiscard]] constexpr size_t material() const noexcept;

    /** Returns true if there are no pawns of this color anywhere on the given file. */
    [[nodiscard]] constexpr bool is_file_half_open(File file) const noexcept;

    /** Returns true if this side has at least one bishop on each color complex. */
    [[nodiscard]] constexpr bool has_bishop_pair() const noexcept;

    /** Returns the location of the king. */
    [[nodiscard]] constexpr Square get_king_location() const noexcept;

    /** Returns the type of the piece on the given square, or ``nullopt`` if the
        square is empty. Note that libchess's bitboard board representation is
        not optimized for this operation.
     */
    [[nodiscard]] constexpr std::optional<PieceType> get_piece_on(Square square) const noexcept;

    /** Removes the piece on the given square, if any.
        This method asserts if the ``square`` is the location of the king.
     */
    constexpr void capture_at(Square square) noexcept;
};

/** Returns true if any of the ``pieces`` attacks any of the ``targetSquares``.
    This function considers only pseudo-legal moves, not strictly legal moves.

    @tparam Side The color that the ``pieces`` represent.

    @ingroup board
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr bool squares_attacked(
    const Pieces& pieces, Bitboard targetSquares, Bitboard enemyPieces) noexcept;

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

constexpr Bitboard& Pieces::get_type(const PieceType type) noexcept
{
    switch (type) {
        case PieceType::Knight: return knights;
        case PieceType::Bishop: return bishops;
        case PieceType::Rook  : return rooks;
        case PieceType::Queen : return queens;
        case PieceType::King  : return king;
        default               : return pawns;
    }
}

constexpr Bitboard Pieces::get_type(const PieceType type) const noexcept
{
    switch (type) {
        case PieceType::Knight: return knights;
        case PieceType::Bishop: return bishops;
        case PieceType::Rook  : return rooks;
        case PieceType::Queen : return queens;
        case PieceType::King  : return king;
        default               : return pawns;
    }
}

constexpr Bitboard Pieces::occupied() const noexcept
{
    return pawns | knights | bishops | rooks | queens | king;
}

constexpr size_t Pieces::material() const noexcept
{
    namespace values = pieces::values;

    return (pawns.count() * values::PAWN)
         + (knights.count() * values::KNIGHT)
         + (bishops.count() * values::BISHOP)
         + (rooks.count() * values::ROOK)
         + (queens.count() * values::QUEEN);
}

constexpr bool Pieces::is_file_half_open(const File file) const noexcept
{
    return (pawns & masks::files::get(file)).none();
}

constexpr bool Pieces::has_bishop_pair() const noexcept
{
    return (bishops & masks::LIGHT_SQUARES).any()
        && (bishops & masks::DARK_SQUARES).any();
}

constexpr Square Pieces::get_king_location() const noexcept
{
    assert(king.count() == 1uz);

    return Square::from_index(king.first());
}

constexpr std::optional<PieceType> Pieces::get_piece_on(const Square square) const noexcept
{
    static constexpr auto allTypes = magic_enum::enum_values<PieceType>();

    if (const auto found = std::ranges::find_if(
            allTypes,
            [this, index = square.index()](const PieceType type) {
                return get_type(type).test(index);
            });
        found != allTypes.end()) {
        return *found;
    }

    return std::nullopt;
}

constexpr void Pieces::capture_at(const Square square) noexcept
{
    const auto idx = square.index();

    // if we're trying to capture the king, then an illegal move has already been played
    assert(! king.test(idx));

    pawns.unset(idx);
    knights.unset(idx);
    bishops.unset(idx);
    rooks.unset(idx);
    queens.unset(idx);
}

template <Color Side>
constexpr bool squares_attacked(
    const Pieces& pieces, const Bitboard targetSquares, const Bitboard enemyPieces) noexcept
{
    namespace move_gen = moves::pseudo_legal;

    const auto friendlyPieces = pieces.occupied();

    const auto pawnAttacks = move_gen::pawn_captures<Side>(pieces.pawns, enemyPieces);

    if ((pawnAttacks & targetSquares).any())
        return true;

    const auto knightAttacks = move_gen::knight(pieces.knights, friendlyPieces);

    if ((knightAttacks & targetSquares).any())
        return true;

    const auto kingAttacks = move_gen::king(pieces.king, friendlyPieces);

    if ((kingAttacks & targetSquares).any())
        return true;

    const auto allOccupied = friendlyPieces | enemyPieces;

    const auto queenAttacks = move_gen::queen(pieces.queens, allOccupied, friendlyPieces);

    if ((queenAttacks & targetSquares).any())
        return true;

    const auto rookAttacks = move_gen::rook(pieces.rooks, allOccupied, friendlyPieces);

    if ((rookAttacks & targetSquares).any())
        return true;

    const auto bishopAttacks = move_gen::bishop(pieces.bishops, allOccupied, friendlyPieces);

    return (bishopAttacks & targetSquares).any();
}

} // namespace chess::board
