/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <format>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace chess::notation {

using board::Square;
using PieceType = pieces::Type;

namespace {

    // returns a vector containing all legal moves for the given
    // piece type that have the same target square as `move`
    [[nodiscard]] std::vector<Move> get_possible_move_origins(
        const Position& position, const Square& targetSquare, const PieceType piece)
    {
        // generate legal moves for given piece type
        auto pieceMoves = moves::generate_for(position, piece);

        // prune moves not to the target square
        std::erase_if(pieceMoves,
            [targetSquare](const Move& candidate) { return candidate.to != targetSquare; });

        return pieceMoves;
    }

    [[nodiscard]] std::string_view get_check_string(const Position& position)
    {
        if (! position.is_check())
            return {};

        if (moves::generate(position).empty())
            return "#"; // checkmate

        return "+"; // check
    }

    [[nodiscard]] std::string get_disambig_string(const Position& position, const Move& move)
    {
        const auto pieceMoves = get_possible_move_origins(position, move.to, move.piece);

        if (pieceMoves.size() < 2uz)
            return {};

        // order of preference for disambiguation:
        // 1. file of departure if different
        // 2. rank of departure if the files are the same but the ranks differ
        // 3. the complete origin square coordinate otherwise

        const auto originSquare = move.from;

        if (std::ranges::count_if(pieceMoves,
                [originSquare](const Move& candidate) { return candidate.from.file == originSquare.file; })
            == 1uz) {
            // file of departure is unique, use it to disambiguate
            return std::format("{}", originSquare.file);
        }

        if (std::ranges::count_if(pieceMoves,
                [originSquare](const Move& candidate) { return candidate.from.rank == originSquare.rank; })
            == 1uz) {
            // rank of departure is unique, use it to disambiguate
            return std::format("{}", originSquare.rank);
        }

        [[unlikely]];

        // use both rank & file (is this ever actually reached??)
        return std::format("{}", originSquare);
    }

} // namespace

std::string to_alg(const Position& position, const Move& move)
{
    if (move.is_castling()) {
        if (move.to.is_kingside())
            return "O-O";

        return "O-O-O";
    }

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

    // with every field: Ngxf4+
    return std::format("{}{}{}{}{}",
        move.piece, get_disambig_string(position, move), captureStr, move.to, get_check_string(position));
}

namespace {

    using board::File;
    using board::Rank;

    [[nodiscard]] Square get_starting_square_from_file(
        const std::span<const Move> possibleOrigins, const File file)
    {
        auto moveStartsOnFile = [file](const Move& move) { return move.from.file == file; };

        if (std::ranges::count_if(possibleOrigins, moveStartsOnFile)
            > 1uz) {
            throw std::invalid_argument {
                std::format(
                    "Disambiguation given file {}, but multiple pieces of this type can move to the target square from this file!",
                    file)
            };
        }

        const auto move = std::ranges::find_if(possibleOrigins, moveStartsOnFile);

        if (move == possibleOrigins.end()) {
            throw std::invalid_argument {
                std::format(
                    "Disambiguation given file {}, but no piece of this type can move to the target square from this file!",
                    file)
            };
        }

        return move->from;
    }

    [[nodiscard]] Square get_starting_square_from_rank(
        const std::span<const Move> possibleOrigins, const Rank rank)
    {
        auto moveStartsOnRank = [rank](const Move& move) { return move.from.rank == rank; };

        if (std::ranges::count_if(possibleOrigins, moveStartsOnRank)
            > 1uz) {
            throw std::invalid_argument {
                std::format(
                    "Disambiguation given rank {}, but multiple pieces of this type can move to the target square from this rank!",
                    rank)
            };
        }

        const auto move = std::ranges::find_if(possibleOrigins, moveStartsOnRank);

        if (move == possibleOrigins.end()) {
            throw std::invalid_argument {
                std::format(
                    "Disambiguation given rank {}, but no piece of this type can move to the target square from this rank!",
                    rank)
            };
        }

        return move->from;
    }

    [[nodiscard]] Square get_starting_square(
        const Position& position, const Square& targetSquare, const PieceType piece,
        const std::string_view text)
    {
        const auto possibleOrigins = get_possible_move_origins(position, targetSquare, piece);

        if (possibleOrigins.empty())
            throw std::invalid_argument {
                std::format("No piece of type {} can legally reach square {}", piece, targetSquare)
            };

        if (possibleOrigins.size() == 1uz)
            return possibleOrigins.front().from;

        if (text.empty())
            throw std::invalid_argument {
                std::format("Multiple pieces of type {} can legally reach square {}, but no disambiguation string was provided",
                    piece, targetSquare)
            };

        if (text.length() > 1uz)
            return Square::from_string(text);

        switch (text.front()) {
            case 'a': [[fallthrough]];
            case 'A':
                return get_starting_square_from_file(possibleOrigins, File::A);

            case 'b': [[fallthrough]];
            case 'B':
                return get_starting_square_from_file(possibleOrigins, File::B);

            case 'c': [[fallthrough]];
            case 'C':
                return get_starting_square_from_file(possibleOrigins, File::C);

            case 'd': [[fallthrough]];
            case 'D':
                return get_starting_square_from_file(possibleOrigins, File::D);

            case 'e': [[fallthrough]];
            case 'E':
                return get_starting_square_from_file(possibleOrigins, File::E);

            case 'f': [[fallthrough]];
            case 'F':
                return get_starting_square_from_file(possibleOrigins, File::F);

            case 'g': [[fallthrough]];
            case 'G':
                return get_starting_square_from_file(possibleOrigins, File::G);

            case 'h': [[fallthrough]];
            case 'H':
                return get_starting_square_from_file(possibleOrigins, File::H);

            case '1': return get_starting_square_from_rank(possibleOrigins, Rank::One);
            case '2': return get_starting_square_from_rank(possibleOrigins, Rank::Two);
            case '3': return get_starting_square_from_rank(possibleOrigins, Rank::Three);
            case '4': return get_starting_square_from_rank(possibleOrigins, Rank::Four);
            case '5': return get_starting_square_from_rank(possibleOrigins, Rank::Five);
            case '6': return get_starting_square_from_rank(possibleOrigins, Rank::Six);
            case '7': return get_starting_square_from_rank(possibleOrigins, Rank::Seven);
            case '8': return get_starting_square_from_rank(possibleOrigins, Rank::Eight);

            default:
                throw std::invalid_argument {
                    std::format("Unrecognized character in disambiguation string: {}", text.front())
                };
        }
    }

} // namespace

Move from_alg(const Position& position, std::string_view text)
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

    // string is of the form Nc6 or Nxc6

    // TODO: Deal with abbreviated pawn moves

    const auto targetSquare = Square::from_string(text.substr(text.length() - 2uz));

    // trim target square
    text.remove_suffix(2uz);

    // trim trailing x
    if (text.back() == 'x')
        text.remove_suffix(1uz);

    const auto pieceType = pieces::from_string(text.substr(0uz, 1uz));

    // trim piece type
    text = text.substr(1uz);

    return {
        .from  = get_starting_square(position, targetSquare, pieceType, text),
        .to    = targetSquare,
        .piece = pieceType
    };
}

} // namespace chess::notation
