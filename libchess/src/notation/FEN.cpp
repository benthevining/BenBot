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

#include "FENHelpers.hpp"
#include <cstddef> // IWYU pragma: keep - for size_t
#include <expected>
#include <format>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <string>

namespace chess::notation {

using pieces::Color;
using std::size_t;

std::string to_fen(const Position& position, const bool alwaysWriteEPSqare)
{
    std::string fen;

    fen_helpers::write_piece_positions(position, fen);

    fen.push_back(' ');

    // side to move
    fen.push_back(
        position.is_white_to_move() ? 'w' : 'b');

    fen.push_back(' ');

    fen_helpers::write_castling_rights(
        position.whiteCastlingRights, position.blackCastlingRights, fen);

    fen.push_back(' ');

    const auto epMoves = position.is_white_to_move()
                           ? moves::detail::get_en_passant<Color::White>(position)
                           : moves::detail::get_en_passant<Color::Black>(position);

    if (alwaysWriteEPSqare or not epMoves.empty()) {
        fen_helpers::write_en_passant_target_square(
            position.enPassantTargetSquare, fen);
    } else {
        fen.push_back('-');
    }

    fen.push_back(' ');

    util::write_integer<3uz>(position.halfmoveClock, fen);

    fen.push_back(' ');

    util::write_integer<4uz>(position.fullMoveCounter, fen);

    return fen;
}

using PositionOrError = std::expected<Position, std::string>;

PositionOrError from_fen(std::string_view fenString)
{
    using util::int_from_string;
    using util::split_at_first_space;

    fenString = util::trim(fenString);

    if (fenString.empty())
        return std::unexpected("Cannot parse Position from empty FEN string");

    auto position = Position::empty();

    const auto [piecePositions, rest1] = split_at_first_space(fenString);

    return fen_helpers::parse_piece_positions(piecePositions, position)
        .and_then([rest1, &position]() -> PositionOrError {
            const auto [sideToMove, rest2] = split_at_first_space(rest1);

            return fen_helpers::parse_side_to_move(sideToMove, position)
                .and_then([rest2, &position]() -> PositionOrError {
                    const auto [castlingRights, rest3] = split_at_first_space(rest2);

                    fen_helpers::parse_castling_rights(castlingRights, position);

                    const auto [epTarget, rest4] = split_at_first_space(rest3);

                    fen_helpers::parse_en_passant_target_square(epTarget, position);

                    // tolerate the final 2 fields being omitted
                    if (not util::trim(rest4).empty()) {
                        const auto [halfMoveClock, fullMoveCounter] = split_at_first_space(rest4);

                        position.halfmoveClock   = int_from_string(halfMoveClock, position.halfmoveClock);
                        position.fullMoveCounter = int_from_string(fullMoveCounter, position.fullMoveCounter);
                    }

                    position.refresh_zobrist();

                    return position;
                });
        });
}

} // namespace chess::notation
