/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include "FENHelpers.hpp" // NOLINT(build/include_subdir)
#include <format>
#include <libchess/notation/EPD.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/util/Strings.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

namespace chess::notation {

using pieces::Color;

EPDPosition from_epd(std::string_view epdString)
{
    using util::split_at_first_space;

    epdString = util::trim(epdString);

    if (epdString.empty()) {
        throw std::invalid_argument {
            "Cannot parse Position from empty FEN string"
        };
    }

    EPDPosition pos {
        .position = Position::empty()
    };

    const auto [piecePositions, rest1] = split_at_first_space(epdString);

    fen_helpers::parse_piece_positions(piecePositions, pos.position);

    const auto [sideToMove, rest2] = split_at_first_space(rest1);

    fen_helpers::parse_side_to_move(sideToMove, pos.position);

    const auto [castlingRights, rest3] = split_at_first_space(rest2);

    fen_helpers::parse_castling_rights(castlingRights, pos.position);

    const auto [epTarget, rest4] = split_at_first_space(rest3);

    fen_helpers::parse_en_passant_target_square(epTarget, pos.position);

    // TODO: operations

    return pos;
}

namespace {

    void write_operations(
        const EPDPosition& pos, std::string& output)
    {
        for (const auto& [key, value] : pos.operations)
            output.append(std::format(" {} \"{}\";", key, value));

        using namespace std::literals::string_literals; // NOLINT

        if (! pos.operations.contains("fmvn"s))
            output.append(std::format(" fmvn {}", pos.position.fullMoveCounter));

        if (! pos.operations.contains("hmvc"s))
            output.append(std::format(" hmvc {}", pos.position.halfmoveClock));
    }

} // namespace

std::string to_epd(const EPDPosition& pos)
{
    std::string epd;

    fen_helpers::write_piece_positions(pos.position, epd);

    epd.push_back(' ');

    // side to move
    epd.push_back(
        pos.position.sideToMove == Color::White ? 'w' : 'b');

    epd.push_back(' ');

    fen_helpers::write_castling_rights(
        pos.position.whiteCastlingRights, pos.position.blackCastlingRights, epd);

    epd.push_back(' ');

    fen_helpers::write_en_passant_target_square(
        pos.position.enPassantTargetSquare, epd);

    epd.push_back(' ');

    write_operations(pos, epd);

    return epd;
}

} // namespace chess::notation
