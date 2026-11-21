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
    This file defines the AttackInfo struct and the compute_attacks() function.
    @ingroup moves
 */

#pragma once

#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::moves {

using board::Bitboard;
using game::Position;
using pieces::Color;

/** The squares attacked by each side's pieces, broken down by piece type.

    These are raw attack sets: squares occupied by friendly pieces are *not* masked out,
    so a set bit on a square holding one of our own pieces means that piece is defended.
    Sliders don't see through other pieces (there are no x-rays).

    @ingroup moves
    @see compute_attacks()
 */
struct AttackInfo final {
    /** The attack sets for one side. */
    struct Side final {
        Bitboard pawns;   ///< Squares attacked by pawns.
        Bitboard knights; ///< Squares attacked by knights.
        Bitboard bishops; ///< Squares attacked by bishops.
        Bitboard rooks;   ///< Squares attacked by rooks.
        Bitboard queens;  ///< Squares attacked by queens.
        Bitboard king;    ///< Squares attacked by the king.

        /** Squares attacked by any of this side's pieces, including the king. */
        Bitboard all;

        /** Squares attacked at least twice by this side's pieces, including the king.
            Two pieces of the same type attacking a square count as two attackers.
         */
        Bitboard twice;

        /** Compares two attack sets for equality. */
        [[nodiscard]] constexpr auto operator==(const Side&) const noexcept -> bool = default;
    };

    /** White's attack sets. */
    Side white;

    /** Black's attack sets. */
    Side black;

    /** Returns the attack sets for the given side. */
    template <Color C>
    [[nodiscard]] constexpr auto side() const noexcept -> const Side&
    {
        if constexpr (C == Color::White)
            return white;
        else
            return black;
    }

    /** Returns the attack sets for the given side. */
    [[nodiscard]] constexpr auto side(const Color color) const noexcept -> const Side&
    {
        return color == Color::White ? white : black;
    }

    /** Compares two AttackInfo objects for equality. */
    [[nodiscard]] constexpr auto operator==(const AttackInfo&) const noexcept -> bool = default;
};

/** Calculates the attack sets for both sides in the given position.

    The ``onPieceAttacks`` callback is called once for each knight, bishop, rook and queen
    on the board, as ``onPieceAttacks(color, pieceType, attacks, info)``, where ``attacks``
    is that one piece's raw attack set. This lets an eval term that needs per-piece attacks
    (such as mobility) share the same pass over the pieces, instead of calculating them again.
    When the callback is called, ``info`` already holds both sides' pawn & king attacks, but
    the other fields (including ``all`` & ``twice``) are still being filled in.

    @ingroup moves
 */
template <typename Callback>
[[nodiscard]] auto compute_attacks(
    const Position& position, Callback&& onPieceAttacks)
    -> AttackInfo;

/** Calculates the attack sets for both sides in the given position.

    @ingroup moves
 */
[[nodiscard]] inline auto compute_attacks(
    const Position& position)
    -> AttackInfo;

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

#ifndef DOXYGEN
namespace detail {

    template <Color Side, typename Callback>
    void compute_piece_attacks(
        const Position& position, const AttackInfo& info,
        AttackInfo::Side& result, Callback& onPieceAttacks)
    {
        using PieceType = pieces::Type;

        const auto& pieces = position.pieces_for<Side>();

        const auto occupied = position.occupied();

        // each piece's attacks are added to the running totals one at a time,
        // so that two pieces of the same type attacking a square show up in twice
        auto add_attacks = [&result](const Bitboard attacks) noexcept {
            result.twice |= result.all & attacks;
            result.all |= attacks;
        };

        for (const auto square : pieces.knights.squares()) {
            const auto attacks = patterns::knight(Bitboard::from_square(square));
            add_attacks(attacks);
            result.knights |= attacks;
            onPieceAttacks(Side, PieceType::Knight, attacks, info);
        }

        for (const auto square : pieces.bishops.squares()) {
            const auto attacks = magics::bishop(square, occupied, { });
            add_attacks(attacks);
            result.bishops |= attacks;
            onPieceAttacks(Side, PieceType::Bishop, attacks, info);
        }

        for (const auto square : pieces.rooks.squares()) {
            const auto attacks = magics::rook(square, occupied, { });
            add_attacks(attacks);
            result.rooks |= attacks;
            onPieceAttacks(Side, PieceType::Rook, attacks, info);
        }

        for (const auto square : pieces.queens.squares()) {
            const auto attacks = magics::queen(square, occupied, { });
            add_attacks(attacks);
            result.queens |= attacks;
            onPieceAttacks(Side, PieceType::Queen, attacks, info);
        }
    }

} // namespace detail
#endif

template <typename Callback>
auto compute_attacks(
    const Position& position, Callback&& onPieceAttacks)
    -> AttackInfo
{
    AttackInfo info;

    info.white.pawns = patterns::pawn_attacks<Color::White>(position.whitePieces.pawns);
    info.black.pawns = patterns::pawn_attacks<Color::Black>(position.blackPieces.pawns);

    info.white.king = patterns::king(position.whitePieces.king);
    info.black.king = patterns::king(position.blackPieces.king);

    // squares attacked by two pawns (one capturing east & one capturing west), or by a pawn & the king
    info.white.twice = board::shifts::pawn_capture_east<Color::White>(position.whitePieces.pawns)
                     & board::shifts::pawn_capture_west<Color::White>(position.whitePieces.pawns);
    info.black.twice = board::shifts::pawn_capture_east<Color::Black>(position.blackPieces.pawns)
                     & board::shifts::pawn_capture_west<Color::Black>(position.blackPieces.pawns);

    info.white.twice |= info.white.pawns & info.white.king;
    info.black.twice |= info.black.pawns & info.black.king;

    info.white.all = info.white.pawns | info.white.king;
    info.black.all = info.black.pawns | info.black.king;

    detail::compute_piece_attacks<Color::White>(position, info, info.white, onPieceAttacks);
    detail::compute_piece_attacks<Color::Black>(position, info, info.black, onPieceAttacks);

    return info;
}

inline auto compute_attacks(
    const Position& position)
    -> AttackInfo
{
    return compute_attacks(
        position,
        [](Color, pieces::Type, Bitboard, const AttackInfo&) noexcept { });
}

} // namespace chess::moves
