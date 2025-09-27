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
#include <expected>
#include <format>
#include <iterator>
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
#include <string>
#include <string_view>
#include <utility>

namespace chess::notation {

using board::Square;
using PieceType = pieces::Type;
using std::string;
using std::string_view;

namespace {

    // returns a vector containing all legal moves for the
    // given piece type that have the given target square
    [[nodiscard]] auto get_possible_move_origins(
        const Position& position, const Square& targetSquare, const PieceType piece)
        -> moves::MoveList
    {
        auto moves = moves::generate_for(position, piece);

        // erase moves not to the given target square
        moves.erase(
            std::ranges::remove_if(moves,
                [targetSquare](const Move& candidate) { return candidate.to() != targetSquare; })
                .begin(),
            moves.end());

        return moves;
    }

    [[nodiscard]] auto get_check_string(const Position& position, const Move& move)
        -> string_view
    {
        const auto newPos = after_move(position, move);

        if (not newPos.is_check())
            return {};

        if (not moves::any_legal_moves(newPos))
            return "#"; // checkmate

        return "+"; // check
    }

    [[nodiscard]] auto get_disambig_string(const Position& position, const Move& move)
        -> string
    {
        const auto pieceMoves = get_possible_move_origins(position, move.to(), move.piece());

        if (pieceMoves.size() < 2uz)
            return {};

        // Order of preference for disambiguation:
        // 1. file of departure if different
        // 2. rank of departure

        const auto [file, rank] = move.from();

        if (std::cmp_equal(
                std::ranges::count_if(pieceMoves,
                    [file](const Move& candidate) { return candidate.from().file == file; }),
                1)) {
            return std::format("{}", file);
        }

        assert(std::cmp_equal(
            std::ranges::count_if(pieceMoves,
                [rank](const Move& candidate) { return candidate.from().rank == rank; }),
            1));

        return std::format("{}", rank);
    }

} // namespace

string to_alg(const Position& position, const Move& move)
{
    assert(not move.is_null());

    const auto checkStr = get_check_string(position, move);

    if (move.is_castling()) {
        [[unlikely]];

        static constexpr string_view KINGSIDE_CASTLE { "O-O" };
        static constexpr string_view QUEENSIDE_CASTLE { "O-O-O" };

        return std::format(
            "{}{}",
            move.to().is_kingside() ? KINGSIDE_CASTLE : QUEENSIDE_CASTLE,
            checkStr);
    }

    const bool isCapture = position.is_capture(move);

    if (const auto prom = move.promoted_type()) {
        [[unlikely]];

        if (isCapture)
            return std::format("{}x{}={}{}", move.from().file, move.to(), prom.value(), checkStr);

        return std::format("{}={}{}", move.to(), prom.value(), checkStr);
    }

    if (move.piece() == PieceType::Pawn) {
        if (isCapture)
            return std::format("{}x{}{}", move.from().file, move.to(), checkStr);

        return std::format("{}{}", move.to(), checkStr);
    }

    const auto* captureStr = isCapture ? "x" : "";

    // with every field: Ngxf4+
    return std::format("{}{}{}{}{}",
        move.piece(), get_disambig_string(position, move), captureStr, move.to(), checkStr);
}

namespace {

    using board::File;
    using board::Rank;
    using pieces::Color;
    using MoveSpan      = std::span<const Move>;
    using SquareOrError = std::expected<Square, string>;

    [[nodiscard]] auto get_starting_square_from_file(
        const MoveSpan possibleOrigins, const File file)
        -> SquareOrError
    {
        auto moveStartsOnFile = [file](const Move& move) { return move.from().file == file; };

        if (std::cmp_greater(
                std::ranges::count_if(possibleOrigins, moveStartsOnFile),
                1)) {
            return std::unexpected(
                std::format(
                    "Disambiguation given file {}, but multiple pieces of this type can move to the target square from this file!",
                    file));
        }

        const auto move = std::ranges::find_if(possibleOrigins, moveStartsOnFile);

        if (move == possibleOrigins.end()) {
            return std::unexpected(
                std::format(
                    "Disambiguation given file {}, but no piece of this type can move to the target square from this file!",
                    file));
        }

        return move->from();
    }

    [[nodiscard]] auto get_starting_square_from_rank(
        const MoveSpan possibleOrigins, const Rank rank)
        -> SquareOrError
    {
        auto moveStartsOnRank = [rank](const Move& move) { return move.from().rank == rank; };

        if (std::cmp_greater(
                std::ranges::count_if(possibleOrigins, moveStartsOnRank),
                1)) {
            return std::unexpected(
                std::format(
                    "Disambiguation given rank {}, but multiple pieces of this type can move to the target square from this rank!",
                    rank));
        }

        const auto move = std::ranges::find_if(possibleOrigins, moveStartsOnRank);

        if (move == possibleOrigins.end()) {
            return std::unexpected(
                std::format(
                    "Disambiguation given rank {}, but no piece of this type can move to the target square from this rank!",
                    rank));
        }

        return move->from();
    }

    [[nodiscard]] auto get_starting_square(
        const Position& position, const Square& targetSquare, const PieceType piece,
        const string_view text)
        -> SquareOrError
    {
        const auto possibleOrigins = get_possible_move_origins(position, targetSquare, piece);

        if (possibleOrigins.empty()) {
            return std::unexpected(
                std::format(
                    "No piece of type {} can legally reach square {}",
                    piece, targetSquare));
        }

        if (possibleOrigins.size() == 1uz)
            return possibleOrigins.front().from();

        if (text.empty()) {
            return std::unexpected(
                std::format(
                    "Multiple pieces of type {} can legally reach square {}, but no disambiguation string was provided",
                    piece, targetSquare));
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

            default: {
                return std::unexpected(
                    std::format(
                        "Unrecognized character in disambiguation string: {}",
                        text.front()));
            }
        }
    }

    [[nodiscard]] constexpr auto create_pawn_capture(
        const Square& targetSquare, const File startingFile, const Color color)
        -> Move
    {
        const auto fromRank = color == Color::White
                                ? prev_pawn_rank<Color::White>(targetSquare.rank)
                                : prev_pawn_rank<Color::Black>(targetSquare.rank);

        return {
            Square {
                .file = startingFile,
                .rank = fromRank },
            targetSquare, PieceType::Pawn
        };
    }

    [[nodiscard]] constexpr auto parse_pawn_capture(
        const Square& targetSquare, const string_view startingFileText, const Color color)
        -> std::optional<Move>
    {
        assert(not startingFileText.empty());

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

    [[nodiscard]] constexpr auto parse_promotion(
        const string_view text, const Color color)
        -> std::optional<Move>
    {
        const auto eqSgnPos = text.find('=');

        if (eqSgnPos == string_view::npos)
            return std::nullopt;

        const auto promotedType = pieces::from_string(text.substr(eqSgnPos + 1uz, 1uz));

        if (not promotedType.has_value())
            return std::nullopt;

        if (const auto xPos = text.find('x');
            xPos != string_view::npos) {
            // string is of form dxe8=Q
            const auto file = board::file_from_char(text.at(xPos - 1uz));

            if (not file.has_value())
                return std::nullopt;

            return Move {
                Square {
                    .file = file.value(),
                    .rank = color == Color::White ? Rank::Seven : Rank::Two },
                Square::from_string(text.substr(eqSgnPos - 2uz, 2uz)),
                PieceType::Pawn, promotedType.value()
            };
        }

        // string is of form e8=Q
        const auto file = board::file_from_char(text.front());

        if (not file.has_value())
            return std::nullopt;

        return moves::promotion(
            file.value(), color, promotedType.value());
    }

} // namespace

std::expected<Move, string> from_alg(const Position& position, string_view text)
{
    text = util::trim(text);

    if (text.empty())
        return std::unexpected("Cannot parse Move from empty string");

    if (text.back() == '+' or text.back() == '#')
        text.remove_suffix(1uz);

    if (text.contains("O-O") or text.contains("0-0")) {
        if (text.contains("-O-") or text.contains("-0-"))
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

    if (isCapture and not text.empty())
        if (const auto move = parse_pawn_capture(targetSquare, text, position.sideToMove))
            return *move;

    const auto pieceType = text.empty()
                             ? std::expected<PieceType, string> { PieceType::Pawn }
                             : pieces::from_string(text.substr(0uz, 1uz));

    if (not pieceType.has_value())
        return std::unexpected(pieceType.error());

    // trim piece type
    if (not text.empty())
        text = text.substr(1uz);

    return get_starting_square(position, targetSquare, pieceType.value(), text)
        .transform([targetSquare, pieceType](const Square startSquare) {
            return Move { startSquare, targetSquare, pieceType.value() };
        });
}

} // namespace chess::notation
