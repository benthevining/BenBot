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
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::board {

using pieces::Color;
using std::size_t;

/** This class encapsulates a bitboard for each piece type.
    An instance of this class encodes the locations of all pieces for one side.

    @ingroup board
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

constexpr Bitboard Pieces::occupied() const noexcept
{
    auto ret = pawns;

    ret |= knights;
    ret |= bishops;
    ret |= rooks;
    ret |= queens;
    ret |= king;

    return ret;
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

} // namespace chess::board
