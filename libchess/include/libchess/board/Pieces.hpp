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

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/File.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::board {

using pieces::Color;
using std::size_t;

using PieceType = pieces::Type;

/** This class encapsulates a bitboard for each piece type.
    An instance of this class encodes the locations of all pieces for one side.

    @ingroup board

    @todo func to get piece type on square (return optional)
    @todo pawn front & rear fills
    @todo func to check for doubled pawns
 */
struct Pieces final {
    /** Creates a Pieces object encoding the starting position for the given side. */
    explicit constexpr Pieces(Color color) noexcept;

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
        This board will never have more than a single bit set.
     */
    Bitboard king;

    /** Returns true if the two piece sets are identical. */
    [[nodiscard]] constexpr bool operator==(const Pieces&) const noexcept = default;

    /** Returns the bitboard corresponding to the given piece type. */
    [[nodiscard]] constexpr Bitboard& type(PieceType type) noexcept;

    /** Returns the bitboard corresponding to the given piece type. */
    [[nodiscard]] constexpr Bitboard type(PieceType type) const noexcept;

    /** Returns a bitboard that is a union of each individual piece-type bitboard.
        The returned bitboard has a bit set if a piece of any type is on that square.
     */
    [[nodiscard]] constexpr Bitboard occupied() const noexcept;

    /** Returns a bitboard that is the inverse of the bitboard returned by ``occupied()``.
        The returned bitboard has a bit set if no piece of any type is on that square.
     */
    [[nodiscard]] constexpr Bitboard free() const noexcept
    {
        return occupied().inverse();
    }

    /** Returns the sum of the material values for all pieces on this side. */
    [[nodiscard]] constexpr size_t material() const noexcept;

    /** Returns true if there are no pawns of this color anywhere on the given file. */
    [[nodiscard]] constexpr bool is_file_half_open(File file) const noexcept;

    /** Returns true if this side has at least one bishop on each color complex. */
    [[nodiscard]] constexpr bool has_bishop_pair() const noexcept;
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

constexpr Bitboard& Pieces::type(const PieceType type) noexcept
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

constexpr Bitboard Pieces::type(const PieceType type) const noexcept
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

    return (pawns.count() * values::pawn())
         + (knights.count() * values::knight())
         + (bishops.count() * values::bishop())
         + (rooks.count() * values::rook())
         + (queens.count() * values::queen());
}

constexpr bool Pieces::is_file_half_open(const File file) const noexcept
{
    return (pawns & masks::files::get(file)).none();
}

constexpr bool Pieces::has_bishop_pair() const noexcept
{
    return (bishops & masks::light_squares()).any()
        && (bishops & masks::dark_squares()).any();
}

} // namespace chess::board
