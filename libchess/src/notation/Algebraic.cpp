/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <format>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <string>
#include <string_view>

namespace chess::notation {

using board::Square;
using pieces::Color;
using PieceType = pieces::Type;

namespace {

    [[nodiscard]] std::string_view get_check_string(const Position& position)
    {
        if (! position.is_check())
            return {};

        if (moves::generate(position).empty())
            return "#"; // checkmate

        return "+"; // check
    }

    [[nodiscard]] std::string_view get_disambig_string(const Position& position, const Move& move)
    {
        const auto& ourPieces = position.sideToMove == Color::White ? position.whitePieces : position.blackPieces;

        const auto& pieceBB = ourPieces.get_type(move.piece);

        const auto targetSquare = move.to;

        // TODO: disambiguate between pieces of same type that can reach the target square, if needed
        // generate legal moves for given piece type -> list of possible starting squares that could reach target square
    }

} // namespace

std::string to_alg(const Position& position, const Move& move)
{
    if (move.is_castling()) {
        if (move.to.is_kingside())
            return "O-O";

        return "O-O-O";
    }

    const bool isWhite = position.sideToMove == Color::White;

    const auto& ourPieces      = isWhite ? position.whitePieces : position.blackPieces;
    const auto& opponentPieces = isWhite ? position.blackPieces : position.whitePieces;

    const bool isCapture = position.is_capture(move);

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

    const auto* captureStr = isCapture ? "x" : "";

    return std::format("{}{}{}{}{}",
        move.piece, get_disambig_string(position, move), captureStr, move.to, get_check_string(position));
}

Move from_alg(const Position& position, const std::string_view text)
{
    if (text == "O-O" || text == "0-0")
        return moves::castle_kingside(position.sideToMove);

    if (text == "O-O-O" || text == "0-0-0")
        return moves::castle_queenside(position.sideToMove);

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
            position.sideToMove, promotedType);
    }

    // TODO: Deal with abbreviated pawn moves
    // TODO: Deal with disambig strings

    // string is of the form Nc6 or Nxc6

    return {
        .from  = Square {}, // TODO: figure out starting square
        .to    = Square::from_string(text.substr(text.length() - 2uz)),
        .piece = pieces::from_string(text.substr(0uz, 1uz))
    };
}

} // namespace chess::notation
