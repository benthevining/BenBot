/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <format>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/UTF8.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <string>
#include <string_view>

namespace chess::game {

std::string Position::move_to_string(const Move& move) const
{
    if (move.is_castling()) {
        if (move.to.is_kingside())
            return "O-O";

        return "O-O-O";
    }

    const bool isWhite = sideToMove == Color::White;

    const auto& ourPieces      = isWhite ? whitePieces : blackPieces;
    const auto& opponentPieces = isWhite ? blackPieces : whitePieces;

    const bool isCapture = opponentPieces.occupied().test(move.to);

    if (move.is_promotion()) {
        if (isCapture)
            return std::format("{}x{}={}", move.from.file, move.to, *move.promotedType);

        return std::format("{}={}", move.to, *move.promotedType);
    }

    if (move.piece == PieceType::Pawn) {
        if (isCapture)
            return std::format("{}x{}", move.from.file, move.to);

        return std::format("{}", move.to);
    }

    const auto captureStr  = isCapture ? "x" : "";
    const auto disambigStr = ""; // TODO

    return std::format("{}{}{}{}", move.piece, disambigStr, captureStr, move.to);
}

namespace utf8_pieces = pieces::utf8;

/* Example output of empty board:

    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |

    A1 is bottom left, H8 is top right
 */

static constexpr std::string_view separator { "|" };

static constexpr std::string_view emptySquare { " |" };

std::string print_utf8(const Position& position)
{
    const auto& whitePieces = position.whitePieces;
    const auto& blackPieces = position.blackPieces;

    std::string result;

    result.reserve(208uz);

    for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>())) {
        result.append(separator);

        for (const auto file : magic_enum::enum_values<File>()) {
            const board::Square square { file, rank };

            if (const auto piece = whitePieces.get_piece_on(square)) {
                result.append(utf8_pieces::white::get(*piece));
                result.append(separator);
                continue;
            }

            if (const auto piece = blackPieces.get_piece_on(square)) {
                result.append(utf8_pieces::black::get(*piece));
                result.append(separator);
                continue;
            }

            result.append(emptySquare);
        }

        result.append("\n");
    }

    return result;
}

} // namespace chess::game
