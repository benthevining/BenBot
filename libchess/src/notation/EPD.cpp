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
#include <cassert>
#include <expected>
#include <format>
#include <libchess/notation/EPD.hpp>
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace chess::notation {

using std::string;
using std::string_view;

using util::int_from_string;

namespace {

    void parse_operations(
        EPDPosition& pos, const string_view text)
    {
        auto opStrings = util::split_by_delim(util::trim(text), ';')
                       | std::views::transform([](const string_view str) { return util::trim(str); })
                       | std::views::filter([](const string_view str) { return not str.empty(); });

        for (const auto opString : opStrings) {
            auto [key, value] = util::split_at_first_space(opString);

            key   = util::trim(key);
            value = util::trim(value);

            if (value.front() == '"')
                value.remove_prefix(1uz);

            if (value.back() == '"')
                value.remove_suffix(1uz);

            assert(not key.empty());
            assert(not value.empty());

            pos.operations[std::string { key }] = value;

            if (key == "fmvn")
                pos.position.fullMoveCounter = int_from_string(value, pos.position.fullMoveCounter);
            else if (key == "hmvc")
                pos.position.halfmoveClock = int_from_string(value, pos.position.halfmoveClock);
        }
    }

} // namespace

using PositionOrError = std::expected<EPDPosition, string>;

PositionOrError from_epd(string_view epdString)
{
    using util::split_at_first_space;

    epdString = util::trim(epdString);

    if (epdString.empty())
        return std::unexpected("Cannot parse Position from empty EPD string");

    EPDPosition pos {
        .position   = Position::empty(),
        .operations = {}
    };

    const auto [piecePositions, rest1] = split_at_first_space(epdString);

    return fen_helpers::parse_piece_positions(piecePositions, pos.position)
        .and_then([rest1, &pos]() -> PositionOrError {
            const auto [sideToMove, rest2] = split_at_first_space(rest1);

            return fen_helpers::parse_side_to_move(sideToMove, pos.position)
                .and_then([rest2, &pos]() -> PositionOrError {
                    const auto [castlingRights, rest3] = split_at_first_space(rest2);

                    fen_helpers::parse_castling_rights(castlingRights, pos.position);

                    const auto [epTarget, rest4] = split_at_first_space(rest3);

                    fen_helpers::parse_en_passant_target_square(epTarget, pos.position);

                    parse_operations(pos, rest4);

                    pos.position.refresh_zobrist();

                    return pos;
                });
        });
}

std::vector<EPDPosition> parse_all_epds(const string_view fileContent)
{
    return util::lines_view(fileContent)
         | std::views::filter([](const string_view line) { return not line.empty(); })
         | std::views::transform([](const string_view line) { return from_epd(line); })
         | std::views::filter([](const PositionOrError& pos) { return pos.has_value(); })
         | std::views::transform([](const PositionOrError& pos) { return pos.value(); })
         | std::ranges::to<std::vector>();
}

namespace {

    void write_operations(
        const EPDPosition& pos, string& output)
    {
        for (const auto& [key, value] : pos.operations)
            output.append(std::format(" {} \"{}\";", key, value));

        if (not pos.operations.contains("fmvn"))
            output.append(std::format(" fmvn {}", pos.position.fullMoveCounter));

        if (not pos.operations.contains("hmvc"))
            output.append(std::format(" hmvc {}", pos.position.halfmoveClock));
    }

} // namespace

string to_epd(const EPDPosition& pos)
{
    string epd;

    fen_helpers::write_piece_positions(pos.position, epd);

    epd.push_back(' ');

    // side to move
    epd.push_back(
        pos.position.is_white_to_move() ? 'w' : 'b');

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
