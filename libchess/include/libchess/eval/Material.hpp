/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines a material evaluation function.
    @ingroup eval
 */

#pragma once

#include <libchess/board/Pieces.hpp>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/PieceTypes.hpp>

namespace chess::eval {

using game::Position;
using PieceType = pieces::Type;

/** This namespace defines piece material values.
    @ingroup eval
 */
namespace piece_values {

    /// @ingroup eval
    /// @{

    /** The value of a pawn. */
    static constexpr auto PAWN { static_cast<Value>(100) };

    /** The value of a knight */
    static constexpr auto KNIGHT { static_cast<Value>(320) };

    /** The value of a bishop. */
    static constexpr auto BISHOP { static_cast<Value>(330) };

    /** The value of a rook. */
    static constexpr auto ROOK { static_cast<Value>(500) };

    /** The value of a queen. */
    static constexpr auto QUEEN { static_cast<Value>(900) };

    /** The value of the king.
        This value isn't really used in counting material, it's just an
        arbitrarily large placeholder value.
     */
    static constexpr auto KING { static_cast<Value>(10000) };

    /// @}

    /** Returns the value of the given piece type.
        @ingroup eval
     */
    [[nodiscard, gnu::const]] constexpr Value get(PieceType type) noexcept;

} // namespace piece_values

/** Returns an aggregate score for the material on the board, relative
    to the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr Value score_material(const Position& position) noexcept;

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

    constexpr Value get(const PieceType type) noexcept
    {
        switch (type) {
            case PieceType::Pawn  : return PAWN;
            case PieceType::Knight: return KNIGHT;
            case PieceType::Bishop: return BISHOP;
            case PieceType::Rook  : return ROOK;
            case PieceType::Queen : return QUEEN;
            default               : return KING;
        }
    }

} // namespace piece_values

constexpr Value score_material(const Position& position) noexcept
{
    auto score_side_material = [](const board::Pieces& pieces) {
        return (static_cast<Value>(pieces.pawns.count()) * piece_values::PAWN)
             + (static_cast<Value>(pieces.knights.count()) * piece_values::KNIGHT)
             + (static_cast<Value>(pieces.bishops.count()) * piece_values::BISHOP)
             + (static_cast<Value>(pieces.rooks.count()) * piece_values::ROOK)
             + (static_cast<Value>(pieces.queens.count()) * piece_values::QUEEN);
    };

    return score_side_material(position.our_pieces()) - score_side_material(position.their_pieces());
}

} // namespace chess::eval
