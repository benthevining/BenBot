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

using std::string;

namespace {
    // Func is a callable that takes an argument of type Square
    // and must return the text to go inside that square, or
    // a space if it's empty
    template <bool IncludeLabels, typename Func>
    [[nodiscard, gnu::cold]] auto generate_board_string(
        Func getSquareText)
        -> string
    {
        string result;

        for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>())) {
            result.append(1uz, '|');

            for (const auto file : magic_enum::enum_values<board::File>()) {
                const board::Square square { .file = file, .rank = rank };

                if constexpr (std::is_same_v<char, std::invoke_result_t<Func, board::Square>>) {
                    result.append(1uz, getSquareText(square));
                } else {
                    result.append(getSquareText(square));
                }

                result.append(1uz, '|');
            }

            if constexpr (IncludeLabels) {
                result.append(1uz, ' ');
                result.append(1uz, rank_to_char(rank));
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
    using board::Square;

    auto print_utf8(const Position& position) -> string
    {
        namespace utf8_pieces = pieces::utf8;

        return generate_board_string<true>(
            [&position](const Square square) {
                return position.whitePieces.get_piece_on(square)
                    .transform([](const PieceType type) { return utf8_pieces::white::get(type); })
                    .or_else([&position, square] {
                        return position.blackPieces.get_piece_on(square)
                            .transform([](const PieceType type) { return utf8_pieces::black::get(type); });
                    })
                    .value_or(std::string_view { " " });
            });
    }

    auto print_ascii(const Position& position) -> string
    {
        return generate_board_string<true>(
            [&position](const Square square) {
                return position.whitePieces.get_piece_on(square)
                    .transform([](const PieceType type) { return pieces::to_char(type, true); })
                    .or_else([&position, square] {
                        return position.blackPieces.get_piece_on(square)
                            .transform([](const PieceType type) { return pieces::to_char(type, false); });
                    })
                    .value_or(' ');
            });
    }

} // namespace game

} // namespace chess
