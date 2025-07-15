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
#include <libchess/pieces/UTF8.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <string>
#include <string_view>

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

namespace chess {

namespace {
    // Func is a callable that takes an argument of type Square
    // and must return the text to go inside that square, or
    // a space if it's empty
    template <typename Func>
    [[nodiscard]] std::string generate_board_string(
        Func       getSquareText,
        const bool includeLabels)
    {
        std::string result;

        for (const auto rank : std::views::reverse(magic_enum::enum_values<board::Rank>())) {
            result.append(1uz, '|');

            for (const auto file : magic_enum::enum_values<board::File>()) {
                const board::Square square { .file = file, .rank = rank };

                result.append(getSquareText(square));
                result.append(1uz, '|');
            }

            if (includeLabels) {
                result.append(1uz, ' ');
                result.append(1uz, rank_to_char(rank));
            }

            result.append(1uz, '\n');
        }

        if (includeLabels)
            result.append(" a b c d e f g h");

        return result;
    }
} // namespace

namespace board {
    std::string print_ascii(const Bitboard board)
    {
        return generate_board_string(
            [board](const Square& square) {
                if (board.test(square))
                    return "x";

                return " ";
            },
            false);
    }
} // namespace board

namespace game {
    std::string print_utf8(const Position& position)
    {
        using board::Square;
        namespace utf8_pieces = pieces::utf8;

        return generate_board_string(
            [&position](const Square& square) {
                if (const auto piece = position.whitePieces.get_piece_on(square))
                    return utf8_pieces::white::get(*piece);

                if (const auto piece = position.blackPieces.get_piece_on(square))
                    return utf8_pieces::black::get(*piece);

                return std::string_view { " " };
            },
            true);
    }
} // namespace game

} // namespace chess
