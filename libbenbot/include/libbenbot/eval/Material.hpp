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

#include <algorithm>
#include <libchess/board/Pieces.hpp>
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
    static constexpr auto PAWN { 100 };

    /** The value of a knight */
    static constexpr auto KNIGHT { 320 };

    /** The value of a bishop. */
    static constexpr auto BISHOP { 330 };

    /** The value of a rook. */
    static constexpr auto ROOK { 500 };

    /** The value of a queen. */
    static constexpr auto QUEEN { 900 };

    /** The value of the king.
        This value isn't really used in counting material, it's just an
        arbitrarily large placeholder value.
     */
    static constexpr auto KING { 10000 };

    /// @}

    /** Returns the value of the given piece type.
        @ingroup eval
     */
    [[nodiscard, gnu::const]] constexpr int get(PieceType type) noexcept;

} // namespace piece_values

/** Returns an aggregate score for the material on the board, relative
    to the side to move.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr int score_material(const Position& position) noexcept;

/** Returns a value between 0 and 1 indicating the approximated game phase.
    The value is 0 at the start of the game and 1 in the late endgame.
    This value is computed based on the amount of non-pawn material left
    on the board.

    @ingroup eval
 */
[[nodiscard, gnu::const]] constexpr float endgame_phase_weight(const Position& position) noexcept;

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

    constexpr int get(const PieceType type) noexcept
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

namespace detail {

    [[nodiscard, gnu::const]] constexpr int count_material(
        const board::Pieces& pieces, const bool includePawns = true) noexcept
    {
        auto total = (static_cast<int>(pieces.knights.count()) * piece_values::KNIGHT)
                   + (static_cast<int>(pieces.bishops.count()) * piece_values::BISHOP)
                   + (static_cast<int>(pieces.rooks.count()) * piece_values::ROOK)
                   + (static_cast<int>(pieces.queens.count()) * piece_values::QUEEN);

        if (includePawns)
            total += (static_cast<int>(pieces.pawns.count()) * piece_values::PAWN);

        return total;
    }

} // namespace detail

constexpr int score_material(const Position& position) noexcept
{
    return detail::count_material(position.our_pieces()) - detail::count_material(position.their_pieces());
}

constexpr float endgame_phase_weight(const Position& position) noexcept
{
    // game phase is roughly determined based on the total amount of non-pawn material left on the board
    // we say that the endgame has begun once the queens & two pairs of minor pieces have been traded
    static constexpr auto EG_MATERIAL_START_ONE_SIDE = static_cast<float>(
        piece_values::ROOK * 2uz + piece_values::BISHOP + piece_values::KNIGHT);

    static constexpr auto EG_MATERIAL_START = EG_MATERIAL_START_ONE_SIDE * 2.f;

    static constexpr auto multiplier = 1.f / EG_MATERIAL_START;

    const auto nonPawnMaterialLeft = detail::count_material(position.whitePieces, false)
                                   + detail::count_material(position.blackPieces, false);

    const auto pcntLeft = static_cast<float>(nonPawnMaterialLeft) * multiplier;

    return 1.f - std::min(1.f, pcntLeft);
}

} // namespace chess::eval
