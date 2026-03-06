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
    This file defines a material evaluation function.
    @ingroup eval
 */

#pragma once

#include <cstdint>
#include <libbenbot/eval/Score.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <utility> // IWYU pragma: keep - for std::unreachable()

namespace ben_bot::eval {

using chess::game::Position;
using PieceType = chess::pieces::Type;

/** This namespace defines piece material values.
    @ingroup eval
 */
namespace piece_values {

    /// @ingroup eval
    /// @{

    /** The value of a pawn. */
    inline constexpr Value PAWN { UINT16_C(100) };

    /** The value of a knight */
    inline constexpr Value KNIGHT { UINT16_C(320) };

    /** The value of a bishop. */
    inline constexpr Value BISHOP { UINT16_C(330) };

    /** The value of a rook. */
    inline constexpr Value ROOK { UINT16_C(500) };

    /** The value of a queen. */
    inline constexpr Value QUEEN { UINT16_C(900) };

    /** The value of the king.
        This value isn't really used in counting material, it's just an
        arbitrarily large placeholder value.
     */
    inline constexpr Value KING { UINT16_C(10000) };

    /// @}

    /** Returns the value of the given piece type.
        @ingroup eval
     */
    [[nodiscard, gnu::const]] constexpr auto get(PieceType type) noexcept -> Value;

} // namespace piece_values

/** Returns an aggregate score for the material on the board, relative
    to the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr auto score_material(const Position& position) noexcept -> Value;

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

namespace piece_values {

    constexpr auto get(const PieceType type) noexcept -> Value
    {
        switch (type) {
            case PieceType::Pawn  : return PAWN;
            case PieceType::Knight: return KNIGHT;
            case PieceType::Bishop: return BISHOP;
            case PieceType::Rook  : return ROOK;
            case PieceType::Queen : return QUEEN;
            case PieceType::King  : return KING;
        }
    }

} // namespace piece_values

namespace detail {

    template <bool IncludePawns = true>
    [[nodiscard, gnu::const]] constexpr auto count_material(
        const chess::board::Pieces& pieces) noexcept
        -> Value
    {
        auto total = (static_cast<Value>(pieces.knights.count()) * piece_values::KNIGHT)
                   + (static_cast<Value>(pieces.bishops.count()) * piece_values::BISHOP)
                   + (static_cast<Value>(pieces.rooks.count()) * piece_values::ROOK)
                   + (static_cast<Value>(pieces.queens.count()) * piece_values::QUEEN);

        if constexpr (IncludePawns) {
            total += (static_cast<Value>(pieces.pawns.count()) * piece_values::PAWN);
        }

        return static_cast<Value>(total);
    }

} // namespace detail

constexpr auto score_material(const Position& position) noexcept -> Value
{
    return detail::count_material(position.our_pieces())
         - detail::count_material(position.their_pieces());
}

} // namespace ben_bot::eval
