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

#include "FENHelpers.hpp" // NOLINT(build/include_subdir)
#include <cstddef>        // IWYU pragma: keep - for size_t
#include <format>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <stdexcept>
#include <string>

namespace chess::notation {

using pieces::Color;
using std::size_t;

std::string to_fen(const Position& position)
{
    std::string fen;

    fen_helpers::write_piece_positions(position, fen);

    fen.push_back(' ');

    // side to move
    fen.push_back(
        position.sideToMove == Color::White ? 'w' : 'b');

    fen.push_back(' ');

    fen_helpers::write_castling_rights(
        position.whiteCastlingRights, position.blackCastlingRights, fen);

    fen.push_back(' ');

    fen_helpers::write_en_passant_target_square(
        position.enPassantTargetSquare, fen);

    fen.push_back(' ');

    util::write_integer<3uz>(position.halfmoveClock, fen);

    fen.push_back(' ');

    util::write_integer<4uz>(position.fullMoveCounter, fen);

    return fen;
}

Position from_fen(std::string_view fenString)
{
    using util::int_from_string;
    using util::split_at_first_space;

    fenString = util::trim(fenString);

    if (fenString.empty()) {
        throw std::invalid_argument {
            "Cannot parse Position from empty FEN string"
        };
    }

    auto position = Position::empty();

    const auto [piecePositions, rest1] = split_at_first_space(fenString);

    fen_helpers::parse_piece_positions(piecePositions, position);

    const auto [sideToMove, rest2] = split_at_first_space(rest1);

    fen_helpers::parse_side_to_move(sideToMove, position);

    const auto [castlingRights, rest3] = split_at_first_space(rest2);

    fen_helpers::parse_castling_rights(castlingRights, position);

    const auto [epTarget, rest4] = split_at_first_space(rest3);

    fen_helpers::parse_en_passant_target_square(epTarget, position);

    const auto [halfMoveClock, fullMoveCounter] = split_at_first_space(rest4);

    position.halfmoveClock   = int_from_string(halfMoveClock, position.halfmoveClock);
    position.fullMoveCounter = int_from_string(fullMoveCounter, position.fullMoveCounter);

    position.refresh_zobrist();

    return position;
}

} // namespace chess::notation
