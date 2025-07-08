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

#include <algorithm>
#include <cassert>
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
#include <libchess/util/Strings.hpp>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace chess::notation {

using board::Square;
using PieceType = pieces::Type;
using std::string;
using std::string_view;

namespace {

    // returns a vector containing all legal moves for the given
    // piece type that have the same target square as `move`
    [[nodiscard]] std::vector<Move> get_possible_move_origins(
        const Position& position, const Square& targetSquare, const PieceType piece)
    {
        auto pieceMoves = moves::generate_for(position, piece);

        std::erase_if(pieceMoves,
            [targetSquare](const Move& candidate) { return candidate.to != targetSquare; });

        return pieceMoves;
    }

    [[nodiscard]] string_view get_check_string(const Position& position, const Move& move)
    {
        const auto newPos = after_move(position, move);

        if (! newPos.is_check())
            return {};

        if (! moves::any_legal_moves(newPos))
            return "#"; // checkmate

        return "+"; // check
    }

    [[nodiscard]] string get_disambig_string(const Position& position, const Move& move)
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
                [file = originSquare.file](const Move& candidate) { return candidate.from.file == file; })
            == 1uz) {
            // file of departure is unique, use it to disambiguate
            return std::format("{}", originSquare.file);
        }

        if (std::ranges::count_if(pieceMoves,
                [rank = originSquare.rank](const Move& candidate) { return candidate.from.rank == rank; })
            == 1uz) {
            // rank of departure is unique, use it to disambiguate
            return std::format("{}", originSquare.rank);
        }

        [[unlikely]];

        // use both rank & file (is this ever actually reached??)
        return std::format("{}", originSquare);
    }

} // namespace

string to_alg(const Position& position, const Move& move)
{
    const auto checkStr = get_check_string(position, move);

    if (move.is_castling()) {
        const auto* castleStr = move.to.is_kingside() ? "O-O" : "O-O-O";

        return std::format("{}{}", castleStr, checkStr);
    }

    const bool isCapture = position.is_capture(move);

    if (move.is_promotion()) {
        if (isCapture)
            return std::format("{}x{}={}{}", move.from.file, move.to, *move.promotedType, checkStr);

        return std::format("{}={}{}", move.to, *move.promotedType, checkStr);
    }

    if (move.piece == PieceType::Pawn) {
        if (isCapture)
            return std::format("{}x{}{}", move.from.file, move.to, checkStr);

        return std::format("{}{}", move.to, checkStr);
    }

    const auto* captureStr = isCapture ? "x" : "";

    // with every field: Ngxf4+
    return std::format("{}{}{}{}{}",
        move.piece, get_disambig_string(position, move), captureStr, move.to, checkStr);
}

namespace {

    using board::File;
    using board::Rank;
    using pieces::Color;
    using MoveSpan = std::span<const Move>;

    [[nodiscard]] Square get_starting_square_from_file(
        const MoveSpan possibleOrigins, const File file)
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
        const MoveSpan possibleOrigins, const Rank rank)
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
        const string_view text)
    {
        const auto possibleOrigins = get_possible_move_origins(position, targetSquare, piece);

        if (possibleOrigins.empty()) {
            throw std::invalid_argument {
                std::format("No piece of type {} can legally reach square {}", piece, targetSquare)
            };
        }

        if (possibleOrigins.size() == 1uz)
            return possibleOrigins.front().from;

        if (text.empty()) {
            throw std::invalid_argument {
                std::format("Multiple pieces of type {} can legally reach square {}, but no disambiguation string was provided",
                    piece, targetSquare)
            };
        }

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

    [[nodiscard]] constexpr Move create_pawn_capture(
        const Square& targetSquare, const File startingFile, const Color color)
    {
        const auto fromRank = color == Color::White
                                ? prev_pawn_rank<Color::White>(targetSquare.rank)
                                : prev_pawn_rank<Color::Black>(targetSquare.rank);

        return {
            .from = Square {
                .file = startingFile,
                .rank = fromRank },
            .to    = targetSquare,
            .piece = PieceType::Pawn
        };
    }

    [[nodiscard]] constexpr std::optional<Move> parse_pawn_capture(
        const Square& targetSquare, const string_view startingFileText, const Color color)
    {
        assert(! startingFileText.empty());

        switch (startingFileText.front()) {
            case 'a': [[fallthrough]];
            case 'A':
                return create_pawn_capture(targetSquare, File::A, color);

                // NB. upper-case B is reserved for signifying the bishop piece type,
                // otherwise bxc4 is undecidable between pawn capture or bishop capture
            case 'b':
                return create_pawn_capture(targetSquare, File::B, color);

            case 'c': [[fallthrough]];
            case 'C':
                return create_pawn_capture(targetSquare, File::C, color);

            case 'd': [[fallthrough]];
            case 'D':
                return create_pawn_capture(targetSquare, File::D, color);

            case 'e': [[fallthrough]];
            case 'E':
                return create_pawn_capture(targetSquare, File::E, color);

            case 'f': [[fallthrough]];
            case 'F':
                return create_pawn_capture(targetSquare, File::F, color);

            case 'g': [[fallthrough]];
            case 'G':
                return create_pawn_capture(targetSquare, File::G, color);

            case 'h': [[fallthrough]];
            case 'H':
                return create_pawn_capture(targetSquare, File::H, color);

            default:
                return std::nullopt;
        }
    }

    [[nodiscard]] constexpr std::optional<Move> parse_promotion(
        const string_view text, const Color color)
    {
        const auto eqSgnPos = text.find('=');

        if (eqSgnPos == string_view::npos)
            return std::nullopt;

        const auto promotedType = pieces::from_string(text.substr(eqSgnPos + 1uz, 1uz));

        if (const auto xPos = text.find('x');
            xPos != string_view::npos) {
            // string is of form dxe8=Q
            return Move {
                .from = {
                    .file = board::file_from_char(text.at(xPos - 1uz)),
                    .rank = color == Color::White ? Rank::Seven : Rank::Two },
                .to           = Square::from_string(text.substr(eqSgnPos - 2uz, 2uz)),
                .piece        = PieceType::Pawn,
                .promotedType = promotedType
            };
        }

        // string is of form e8=Q
        return moves::promotion(
            board::file_from_char(text.front()),
            color, promotedType);
    }

} // namespace

Move from_alg(const Position& position, string_view text)
{
    text = util::trim(text);

    if (text.empty()) {
        throw std::invalid_argument {
            "Cannot parse Move from empty string"
        };
    }

    if (text.back() == '+' || text.back() == '#')
        text.remove_suffix(1uz);

    if (text.contains("O-O") || text.contains("0-0")) {
        if (text.contains("-O-") || text.contains("-0-"))
            return moves::castle_queenside(position.sideToMove);

        return moves::castle_kingside(position.sideToMove);
    }

    if (const auto move = parse_promotion(text, position.sideToMove))
        return *move;

    const auto targetSquare = Square::from_string(text.substr(text.length() - 2uz));

    // trim target square
    text.remove_suffix(2uz);

    const bool isCapture = [text] {
        if (text.empty())
            return false;

        return text.back() == 'x';
    }();

    if (isCapture)
        text.remove_suffix(1uz);

    // at this point, if text is empty, this an abbreviated pawn move such as "e4", etc.
    // if text is not empty, the first char is either piece type, or in the case of a
    // pawn capture, it's the file letter of the starting square

    if (isCapture && ! text.empty())
        if (const auto move = parse_pawn_capture(targetSquare, text, position.sideToMove))
            return *move;

    const auto pieceType = text.empty()
                             ? PieceType::Pawn
                             : pieces::from_string(text.substr(0uz, 1uz));

    // trim piece type
    if (! text.empty())
        text = text.substr(1uz);

    return {
        .from  = get_starting_square(position, targetSquare, pieceType, text),
        .to    = targetSquare,
        .piece = pieceType
    };
}

} // namespace chess::notation
