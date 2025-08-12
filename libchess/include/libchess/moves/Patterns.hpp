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
    This file provides functions for generating move patterns.

    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/pieces/Colors.hpp>

/** This namespace provides functions for generating possible moves for each piece type
    from a given starting square. These functions are not concerned with captures
    or blocking pieces, they deal purely with the piece's movement mechanics - i.e.,
    "attacks on an otherwise empty board". The returned move sets do not include the
    starting square.

    @ingroup moves
 */
namespace chess::moves::patterns {

using board::Bitboard;
using pieces::Color;

/// @ingroup moves
/// @{

/** Calculates all possible pawn pushes for the given starting position and color. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_pushes(Bitboard starting) noexcept;

/** Calculates all possible pawn double pushes for the given starting position and color. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_double_pushes(Bitboard starting) noexcept;

/** Calculates all squares that are attacked by pawns from the given starting position and color.
    This can be used to calculate possible pawn captures.
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Bitboard pawn_attacks(Bitboard starting) noexcept;

/** Calculates all possible knight moves from the given starting position. */
[[nodiscard, gnu::const]] constexpr Bitboard knight(Bitboard starting) noexcept;

/** Calculates all possible bishop moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard bishop(Bitboard starting) noexcept;

/** Calculates all possible rook moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard rook(Bitboard starting) noexcept;

/** Calculates all possible queen moves from the given starting square. */
[[nodiscard, gnu::const]] constexpr Bitboard queen(Bitboard starting) noexcept;

/** Calculates all possible king moves from the given starting position.
    Typically the starting bitboard will have only a single bit set, but this method can
    technically calculate moves for both kings at once, if both of their positions are
    set in the bitboard sent to this function.
 */
[[nodiscard, gnu::const]] constexpr Bitboard king(Bitboard starting) noexcept;

/// @}

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

template <Color Side>
constexpr Bitboard pawn_pushes(const Bitboard starting) noexcept
{
    return board::shifts::pawn_forward<Side>(starting);
}

template <Color Side>
constexpr Bitboard pawn_double_pushes(const Bitboard starting) noexcept
{
    namespace rank_masks = board::masks::ranks;

    if constexpr (Side == Color::White) {
        return (starting << 16uz) // north 2 ranks
             & rank_masks::FOUR;
    } else {
        return (starting >> 16uz) // south 2 ranks
             & rank_masks::FIVE;
    }
}

template <Color Side>
constexpr Bitboard pawn_attacks(const Bitboard starting) noexcept
{
    namespace shifts = board::shifts;

    return shifts::pawn_capture_east<Side>(starting) | shifts::pawn_capture_west<Side>(starting);
}

constexpr Bitboard knight(const Bitboard starting) noexcept
{
    namespace file_masks = board::masks::files;

    constexpr auto notAFile  = file_masks::A.inverse();
    constexpr auto notHFile  = file_masks::H.inverse();
    constexpr auto notABFile = (file_masks::A | file_masks::B).inverse();
    constexpr auto notGHFile = (file_masks::G | file_masks::H).inverse();

    Bitboard moves;

    moves |= (starting & notHFile) << 17uz;  // noNoEa
    moves |= (starting & notGHFile) << 10uz; // noEaEa
    moves |= (starting & notGHFile) >> 6uz;  // soEaEa
    moves |= (starting & notHFile) >> 15uz;  // soSoEa
    moves |= (starting & notAFile) << 15uz;  // noNoWe
    moves |= (starting & notABFile) << 6uz;  // noWeWe
    moves |= (starting & notABFile) >> 10uz; // soWeWe
    moves |= (starting & notAFile) >> 17uz;  // soSoWe

    return moves;
}

constexpr Bitboard bishop(const Bitboard starting) noexcept
{
    const auto diags     = board::fills::diagonal(starting);
    const auto antiDiags = board::fills::antidiagonal(starting);

    return (diags | antiDiags) & starting.inverse();
}

constexpr Bitboard rook(const Bitboard starting) noexcept
{
    const auto ranks = board::fills::rank(starting);
    const auto files = board::fills::file(starting);

    return (ranks | files) & starting.inverse();
}

constexpr Bitboard queen(const Bitboard starting) noexcept
{
    const auto ranks     = board::fills::rank(starting);
    const auto files     = board::fills::file(starting);
    const auto diags     = board::fills::diagonal(starting);
    const auto antiDiags = board::fills::antidiagonal(starting);

    const auto notStartingSquare = starting.inverse();

    return (ranks | files | diags | antiDiags) & notStartingSquare;
}

constexpr Bitboard king(Bitboard starting) noexcept
{
    namespace shifts = board::shifts;

    auto moves = shifts::east(starting) | shifts::west(starting);

    starting |= moves;

    moves |= shifts::north(starting) | shifts::south(starting);

    return moves;
}

} // namespace chess::moves::patterns
