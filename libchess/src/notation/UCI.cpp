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

using MoveOrError = std::expected<Move, std::string>;

MoveOrError from_uci(
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

    if (not from.has_value())
        return std::unexpected(from.error());

    text = text.substr(2uz);

    return Square::from_string(text.substr(0uz, 2uz))
        .and_then([&text, &position, start = from.value()](const Square dest) -> MoveOrError {
            text                 = text.substr(2uz);
            const auto movedType = position.our_pieces().get_piece_on(start);

            if (not movedType.has_value()) {
                [[unlikely]];
                return std::unexpected(
                    std::format(
                        "No piece for color {} can move from square {}",
                        magic_enum::enum_name(position.sideToMove), start));
            }

            // promotion
            if (not text.empty()) {
                [[unlikely]];

                return pieces::from_string(text)
                    .transform([start, dest, type = movedType.value()](const pieces::Type promotedType) {
                        return Move { start, dest, type, promotedType };
                    })
                    .or_else([](const std::string_view parseError) -> MoveOrError {
                        return std::unexpected(
                            std::format(
                                "Error parsing promoted type: {}",
                                parseError));
                    });
            }

            return Move { start, dest, movedType.value() };
        });
}

} // namespace chess::notation
