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

#include <concepts>
#include <format>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libchess/pieces/UTF8.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

/* Example output of empty board with labels:

    | | | | | | | | | 8
    | | | | | | | | | 7
    | | | | | | | | | 6
    | | | | | | | | | 5
    | | | | | | | | | 4
    | | | | | | | | | 3
    | | | | | | | | | 2
    | | | | | | | | | 1
     a b c d e f g h
 */

namespace chess {

using board::Square;
using std::string;
using std::string_view;

namespace {
    template <typename Func>
    concept SquarePrinter = std::regular_invocable<Func, Square>
                        and (std::same_as<std::invoke_result_t<Func, Square>, char>
                             or std::same_as<std::invoke_result_t<Func, Square>, string_view>);

    // Func is a callable that takes an argument of type Square
    // and must return the character to go inside that square,
    // or a space if it's empty
    template <bool IncludeLabels>
    [[nodiscard]] auto generate_board_string(
        SquarePrinter auto getSquareText) -> string
    {
        string result;

        for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>())) {
            for (const auto file : magic_enum::enum_values<board::File>()) {
                result.append(std::format("|{}",
                    getSquareText(Square { .file = file, .rank = rank })));
            }

            result.append(1uz, '|');

            if constexpr (IncludeLabels) {
                result.append(std::format(" {}", rank_to_char(rank)));
            }

            result.append(1uz, '\n');
        }

        if constexpr (IncludeLabels) {
            result.append(" a b c d e f g h");
        }

        return result;
    }
} // namespace

namespace board {
    auto print_ascii(const Bitboard board) -> string
    {
        return generate_board_string<false>(
            [board](const Square square) {
                return board.test(square) ? 'x' : ' ';
            });
    }
} // namespace board

namespace game {
    auto print_utf8(const Position& position) -> string
    {
        namespace utf8_pieces = pieces::utf8;

        return generate_board_string<true>(
            [&position](const Square square) {
                return position.whitePieces.get_piece_on(square)
                    .transform(utf8_pieces::white::get)
                    .or_else([&position, square] {
                        return position.blackPieces.get_piece_on(square)
                            .transform(utf8_pieces::black::get);
                    })
                    .value_or(string_view { " " });
            });
    }

    auto print_ascii(const Position& position) -> string
    {
        return generate_board_string<true>(
            [&position](const Square square) {
                return position.whitePieces.get_piece_on(square)
                    .transform([](const PieceType type) { return to_char(type, true); })
                    .or_else([&position, square] {
                        return position.blackPieces.get_piece_on(square)
                            .transform([](const PieceType type) { return to_char(type, false); });
                    })
                    .value_or(' ');
            });
    }

} // namespace game

} // namespace chess
