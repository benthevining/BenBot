/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <format>
#include <libchess/board/Square.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

namespace chess::notation {

std::string to_uci(const Move& move)
{
    if (move.is_promotion())
        return std::format("{}{}{}", move.from, move.to, *move.promotedType);

    return std::format("{}{}", move.from, move.to);
}

Move from_uci(const Position& position, std::string_view text)
{
    using board::Square;

    Move result;

    result.from = Square::from_string(text.substr(0uz, 2uz));
    text        = text.substr(2uz);

    result.to = Square::from_string(text.substr(0uz, 2uz));
    text      = text.substr(2uz);

    const auto& pieces = position.sideToMove == pieces::Color::White ? position.whitePieces : position.blackPieces;

    const auto movedType = pieces.get_piece_on(result.from);

    if (! movedType.has_value())
        throw std::invalid_argument {
            std::format(
                "No piece for color {} can move from square {}",
                magic_enum::enum_name(position.sideToMove), result.from)
        };

    result.piece = *movedType;

    // promotion
    if (! text.empty())
        result.promotedType = pieces::from_string(text);

    return result;
}

} // namespace chess::notation
