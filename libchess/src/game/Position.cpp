/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <format>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/pieces/UTF8.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace chess::game {

bool Position::is_checkmate() const
{
    return is_check()
        && moves::generate(*this).empty();
}

bool Position::is_stalemate() const
{
    return ! is_check()
        && moves::generate(*this).empty();
}

// TODO: annotate check, checkmate
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

    const bool isCapture = is_capture(move);

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

    const auto captureStr = isCapture ? "x" : "";

    // TODO: disambiguate between pieces of same type that can reach the target square, if needed
    const auto disambigStr = "";

    return std::format("{}{}{}{}", move.piece, disambigStr, captureStr, move.to);
}

Move Position::move_from_string(std::string_view text) const
{
    if (text == "O-O" || text == "0-0")
        return moves::castle_kingside(sideToMove);

    if (text == "O-O-O" || text == "0-0-0")
        return moves::castle_queenside(sideToMove);

    // promotion
    if (const auto eqSgnPos = text.find('=');
        eqSgnPos != std::string_view::npos) {
        const auto promotedType = pieces::from_string(text.substr(eqSgnPos + 1uz, 1uz));

        if (const auto xPos = text.find('x');
            xPos != std::string_view::npos) {
            // string is of form dxe8=Q
            return {
                .from         = board::file_from_char(text.at(xPos - 1uz)),
                .to           = Square::from_string(text.substr(eqSgnPos - 2uz, 2uz)),
                .piece        = PieceType::Pawn,
                .promotedType = promotedType
            };
        }

        // string is of form e8=Q
        return moves::promotion(
            board::file_from_char(text.front()),
            sideToMove, promotedType);
    }

    // TODO: Deal with abbreviated pawn moves

    // string is of the form Nc6 or Nxc6

    return {
        .from  = Square {}, // TODO: figure out starting square
        .to    = Square::from_string(text.substr(text.length() - 2uz)),
        .piece = pieces::from_string(text.substr(0uz, 1uz))
    };

    throw std::invalid_argument {
        std::format("Cannot parse Move from invalid input string: {}", text)
    };
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
    using board::Rank;
    using board::Square;

    const auto& whitePieces = position.whitePieces;
    const auto& blackPieces = position.blackPieces;

    std::string result;

    result.reserve(208uz);

    for (const auto rank : std::views::reverse(magic_enum::enum_values<Rank>())) {
        result.append(separator);

        for (const auto file : magic_enum::enum_values<File>()) {
            const Square square { file, rank };

            if (const auto piece = whitePieces.get_piece_on(square)) {
                result.append(utf8_pieces::white::get(*piece));
                result.append(separator);
                continue;
            }

            if (const auto piece = blackPieces.get_piece_on(square)) {
                result.append(utf8_pieces::black::get(*piece));
                result.append(separator);
            } else {
                result.append(emptySquare);
            }
        }

        result.append("\n");
    }

    return result;
}

} // namespace chess::game
