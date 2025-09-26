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

#include <expected>
#include <format>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/util/Strings.hpp>
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <string_view>

namespace chess::notation {

std::string to_uci(const Move& move)
{
    if (move.is_null()) {
        [[unlikely]];
        return "0000";
    }

    if (const auto prom = move.promoted_type()) {
        [[unlikely]];
        return std::format("{}{}{}",
            move.from(), move.to(),
            pieces::to_char(prom.value(), false));
    }

    return std::format("{}{}", move.from(), move.to());
}

std::expected<Move, std::string> from_uci(
    const Position& position, std::string_view text)
{
    using board::Square;

    text = util::trim(text);

    if (text.empty()) {
        [[unlikely]];
        return std::unexpected("Cannot parse Move from empty string");
    }

    if (text == "0000") {
        [[unlikely]];
        return Move {};
    }

    const auto from = Square::from_string(text.substr(0uz, 2uz));

    text = text.substr(2uz);

    const auto dest = Square::from_string(text.substr(0uz, 2uz));

    text = text.substr(2uz);

    const auto& pieces = position.our_pieces();

    const auto movedType = pieces.get_piece_on(from);

    if (not movedType.has_value()) {
        [[unlikely]];
        return std::unexpected(
            std::format(
                "No piece for color {} can move from square {}",
                magic_enum::enum_name(position.sideToMove), from));
    }

    // promotion
    if (not text.empty()) {
        [[unlikely]];
        return Move {
            from, dest, movedType.value(), pieces::from_string(text)
        };
    }

    return Move { from, dest, movedType.value() };
}

} // namespace chess::notation
