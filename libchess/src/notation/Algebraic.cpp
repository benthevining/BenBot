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

        if (std::cmp_less(pieceMoves.size(), 2))
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

auto to_alg(const Position& position, const Move move) -> string
{
    assert(not move.is_null());

    const auto checkStr = get_check_string(position, move);

    if (move.is_castling()) {
        static constexpr string_view KINGSIDE_CASTLE { "O-O" };
        static constexpr string_view QUEENSIDE_CASTLE { "O-O-O" };

        return std::format(
            "{}{}",
            move.to().is_kingside() ? KINGSIDE_CASTLE : QUEENSIDE_CASTLE,
            checkStr);
    }

    const bool isCapture = position.is_capture(move);

    return move.promoted_type()
        .transform([move, checkStr, isCapture](const PieceType promotedType) {
            if (isCapture)
                return std::format("{}x{}={}{}", move.from().file, move.to(), promotedType, checkStr);

            return std::format("{}={}{}", move.to(), promotedType, checkStr);
        })
        .or_else([move, checkStr, isCapture, &position]() -> std::optional<string> {
            if (move.piece() == PieceType::Pawn) {
                if (isCapture)
                    return std::format("{}x{}{}", move.from().file, move.to(), checkStr);

                return std::format("{}{}", move.to(), checkStr);
            }

            const auto* captureStr = isCapture ? "x" : "";

            // with every field: Ngxf4+
            return std::format("{}{}{}{}{}",
                move.piece(), get_disambig_string(position, move), captureStr, move.to(), checkStr);
        })
        .value();
}

namespace {

    using board::File;
    using board::Rank;
    using pieces::Color;
    using MoveSpan      = std::span<const Move>;
    using SquareOrError = std::expected<Square, string>;
    using MaybeMove     = std::optional<Move>;

    [[nodiscard]] auto get_starting_square_from_file(
        const MoveSpan possibleOrigins, const File file)
        -> SquareOrError
    {
        auto moveStartsOnFile = [file](const Move& move) { return move.from().file == file; };

        if (std::cmp_greater(
                std::ranges::count_if(possibleOrigins, moveStartsOnFile),
                1)) {
            return std::unexpected {
                std::format(
                    "Disambiguation given file {}, but multiple pieces of this type can move to the target square from this file!",
                    file)
            };
        }

        const auto move = std::ranges::find_if(possibleOrigins, moveStartsOnFile);

        if (move == possibleOrigins.end()) {
            return std::unexpected {
                std::format(
                    "Disambiguation given file {}, but no piece of this type can move to the target square from this file!",
                    file)
            };
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
            return std::unexpected {
                std::format(
                    "Disambiguation given rank {}, but multiple pieces of this type can move to the target square from this rank!",
                    rank)
            };
        }

        const auto move = std::ranges::find_if(possibleOrigins, moveStartsOnRank);

        if (move == possibleOrigins.end()) {
            return std::unexpected {
                std::format(
                    "Disambiguation given rank {}, but no piece of this type can move to the target square from this rank!",
                    rank)
            };
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
            return std::unexpected {
                std::format(
                    "No piece of type {} can legally reach square {}",
                    piece, targetSquare)
            };
        }

        if (std::cmp_equal(possibleOrigins.size(), 1))
            return possibleOrigins.front().from();

        if (text.empty()) {
            return std::unexpected {
                std::format(
                    "Multiple pieces of type {} can legally reach square {}, but no disambiguation string was provided",
                    piece, targetSquare)
            };
        }

        if (std::cmp_greater(text.length(), 1))
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
                return std::unexpected {
                    std::format(
                        "Unrecognized character in disambiguation string: {}",
                        text.front())
                };
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
        -> MaybeMove
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
        -> MaybeMove
    {
        const auto eqSgnPos = text.find('=');

        if (eqSgnPos == string_view::npos)
            return std::nullopt;

        return pieces::from_string(text.substr(eqSgnPos + 1uz, 1uz))
            .transform([text, color, eqSgnPos](const PieceType promotedType) -> MaybeMove {
                if (const auto xPos = text.find('x'); xPos != string_view::npos) {
                    // string is of form dxe8=Q
                    return board::file_from_char(text.at(xPos - 1uz))
                        .transform([color, text, eqSgnPos, promotedType](const File file) -> MaybeMove {
                            return Square::from_string(text.substr(eqSgnPos - 2uz, 2uz))
                                .transform([file, color, promotedType](const Square destSquare) -> MaybeMove {
                                    return Move {
                                        Square {
                                            .file = file,
                                            .rank = color == Color::White ? Rank::Seven : Rank::Two },
                                        destSquare, PieceType::Pawn, promotedType
                                    };
                                })
                                .value_or(std::nullopt);
                        })
                        .value_or(std::nullopt);
                }

                // string is of form e8=Q
                return board::file_from_char(text.front())
                    .transform([color, promotedType](const File file) -> MaybeMove {
                        return moves::promotion(file, color, promotedType);
                    })
                    .value_or(std::nullopt);
            })
            .value_or(std::nullopt);
    }

} // namespace

using MoveOrError = std::expected<Move, string>;

auto from_alg(const Position& position, string_view text) -> MoveOrError
{
    text = util::strings::trim(text);

    if (text.empty())
        return std::unexpected { "Cannot parse Move from empty string" };

    if (text.back() == '+' or text.back() == '#')
        text.remove_suffix(1uz);

    if (text.contains("O-O") or text.contains("0-0")) {
        if (text.contains("-O-") or text.contains("-0-"))
            return moves::castle_queenside(position.sideToMove);

        return moves::castle_kingside(position.sideToMove);
    }

    if (const auto move = parse_promotion(text, position.sideToMove))
        return *move;

    return Square::from_string(text.substr(text.length() - 2uz))
        .and_then([&text, &position](const Square targetSquare) -> MoveOrError {
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

            return pieceType
                .and_then([&text, &position, targetSquare](const PieceType type) {
                    // trim piece type
                    if (not text.empty())
                        text = text.substr(1uz);

                    return get_starting_square(position, targetSquare, type, text)
                        .transform([targetSquare, type](const Square startSquare) {
                            return Move { startSquare, targetSquare, type };
                        });
                });
        });
}

} // namespace chess::notation
