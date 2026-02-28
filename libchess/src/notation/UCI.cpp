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
#include <optional>
#include <string>
#include <string_view>

namespace chess::notation {

using std::string;
using std::string_view;

using PieceType = pieces::Type;

inline constexpr string_view UCI_NULL_MOVE { "0000" };

auto to_uci(const Move move) -> string
{
    if (move.is_null()) {
        [[unlikely]];
        return string { UCI_NULL_MOVE };
    }

    return move.promoted_type()
        .transform([move](const PieceType promotedType) {
            return std::format("{}{}{}",
                move.from(), move.to(),
                to_char(promotedType, false));
        })
        .or_else([move] {
            return std::make_optional(
                std::format("{}{}", move.from(), move.to()));
        })
        .value();
}

using MoveOrError = std::expected<Move, string>;

auto from_uci(
    const Position& position, string_view text)
    -> MoveOrError
{
    using board::Square;

    text = util::strings::trim(text);

    if (text.empty())
        return std::unexpected { "Cannot parse UCI move from empty string" };

    if (text == UCI_NULL_MOVE)
        return Move { };

    return Square::from_string(text.substr(0uz, 2uz))
        .and_then([&text, &position](const Square from) {
            text = text.substr(2uz);

            return Square::from_string(text.substr(0uz, 2uz))
                .and_then([&text, &position, from](const Square dest) -> MoveOrError {
                    text = text.substr(2uz);

                    return position.our_pieces()
                        .get_piece_on(from)
                        .transform([text, from, dest](const PieceType movedType) -> MoveOrError {
                            if (text.empty())
                                return Move { from, dest, movedType };

                            // promotion
                            return pieces::from_string(text)
                                .transform([from, dest](const PieceType promotedType) {
                                    return Move { from, dest, PieceType::Pawn, promotedType };
                                })
                                .transform_error([](const string_view parseError) {
                                    return std::format(
                                        "Error parsing promoted type: {}",
                                        parseError);
                                });
                        })
                        .value_or(
                            std::unexpected {
                                std::format(
                                    "No piece for color {} can move from square {}",
                                    magic_enum::enum_name(position.sideToMove), from) });
                });
        });
}

} // namespace chess::notation
