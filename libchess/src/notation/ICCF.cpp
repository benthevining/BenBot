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

#include <cassert>
#include <concepts>
#include <expected>
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/ICCF.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/util/Strings.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace chess::notation {

namespace {
    using board::Square;
    using std::string;
    using PieceType = pieces::Type;

    [[nodiscard, gnu::const]] auto digit_to_char(std::integral auto digit) -> char
    {
        assert(digit >= 0 and digit <= 9);

        return static_cast<char>(
            static_cast<int>('0') + static_cast<int>(digit));
    }

    [[nodiscard]] auto to_iccf_string(const Square square)
        -> string
    {
        return {
            digit_to_char(std::to_underlying(square.file) + 1),
            digit_to_char(std::to_underlying(square.rank) + 1)
        };
    }

    [[nodiscard, gnu::const]] auto to_iccf_char(const PieceType piece) -> char
    {
        switch (piece) {
            case PieceType::Queen : return '1';
            case PieceType::Rook  : return '2';
            case PieceType::Bishop: return '3';
            case PieceType::Knight: return '4';
            case PieceType::Pawn  : [[fallthrough]];
            case PieceType::King  : [[fallthrough]];
            default               : std::unreachable();
        }
    }
} // namespace

auto to_iccf(Move move) -> string
{
    return move.promoted_type()
        .transform([move](const PieceType promotedType) {
            return std::format(
                "{}{}{}",
                to_iccf_string(move.from()),
                to_iccf_string(move.to()),
                to_iccf_char(promotedType));
        })
        .or_else([move] {
            return std::make_optional(
                std::format(
                    "{}{}",
                    to_iccf_string(move.from()),
                    to_iccf_string(move.to())));
        })
        .value();
}

namespace {
    using board::BitboardIndex;

    [[nodiscard, gnu::const]] auto digit_from_char(const char input)
        -> std::expected<BitboardIndex, string>
    {
        switch (input) {
            case '0': return BitboardIndex { 0 };
            case '1': return BitboardIndex { 1 };
            case '2': return BitboardIndex { 2 };
            case '3': return BitboardIndex { 3 };
            case '4': return BitboardIndex { 4 };
            case '5': return BitboardIndex { 5 };
            case '6': return BitboardIndex { 6 };
            case '7': return BitboardIndex { 7 };
            case '8': return BitboardIndex { 8 };
            case '9': return BitboardIndex { 9 };
            default : return std::unexpected {
                std::format("Cannot parse digit from input: {}", input)
            };
        }
    }

    [[nodiscard, gnu::const]] auto parse_square(const std::string_view input)
        -> std::expected<Square, string>
    {
        if (input.size() != 2uz)
            return std::unexpected {
                std::format("Cannot parse Square from string: {}", input)
            };

        return digit_from_char(input.front())
            .and_then([input](const BitboardIndex fileNum) {
                return digit_from_char(input.back())
                    .and_then([fileNum](const BitboardIndex rankNum) {
                        return std::expected<Square, string> {
                            Square {
                                .file = static_cast<board::File>(fileNum - 1),
                                .rank = static_cast<board::Rank>(rankNum - 1) }
                        };
                    });
            });
    }

    [[nodiscard, gnu::const]] auto parse_piece_type(const char input)
        -> std::expected<PieceType, string>
    {
        switch (input) {
            case '1': return PieceType::Queen;
            case '2': return PieceType::Rook;
            case '3': return PieceType::Bishop;
            case '4': return PieceType::Knight;
            default : return std::unexpected {
                std::format("Cannot parse promoted type from input: {}", input)
            };
        }
    }
} // namespace

using MoveOrError = std::expected<Move, string>;

auto from_iccf(
    const Position& position, std::string_view text)
    -> MoveOrError
{
    text = util::strings::trim(text);

    if (text.length() < 4uz)
        return std::unexpected {
            "Expected at least 4 characters for ICCF notation"
        };

    return parse_square(text.substr(0uz, 2uz))
        .and_then([&text, &position](const Square fromSq) {
            text = text.substr(2uz);

            return parse_square(text.substr(0uz, 2uz))
                .and_then([&text, &position, fromSq](const Square toSq) -> MoveOrError {
                    text = text.substr(2uz);

                    return position.our_pieces()
                        .get_piece_on(fromSq)
                        .transform([text, fromSq, toSq](const PieceType movedType) -> MoveOrError {
                            if (text.empty()) {
                                // non-promotion
                                return Move { fromSq, toSq, movedType };
                            }

                            // promotion
                            assert(movedType == PieceType::Pawn);

                            return parse_piece_type(text.front())
                                .transform([fromSq, toSq](const PieceType promotedType) {
                                    return Move { fromSq, toSq, PieceType::Pawn, promotedType };
                                });
                        })
                        .value_or(
                            std::unexpected {
                                std::format(
                                    "No piece for color {} can move from square {}",
                                    magic_enum::enum_name(position.sideToMove), fromSq) });
                });
        });
}

} // namespace chess::notation
