/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/board/Bitboard.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <string>
#include <string_view>

namespace chess::board {

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

    Occupied squares simply get an x inside them
 */
std::string print_ascii(const Bitboard board)
{
    static constexpr std::string_view separator { "|" };
    static constexpr std::string_view emptySquare { " |" };
    static constexpr std::string_view occupiedSquare { "x|" };

    std::string result;

    result.reserve(144uz);

    for (const auto rank : std::views::reverse(magic_enum::enum_values<Rank>())) {
        result.append(separator);

        for (const auto file : magic_enum::enum_values<File>()) {
            if (board.test(Square { .file = file, .rank = rank }))
                result.append(occupiedSquare);
            else
                result.append(emptySquare);
        }

        result.append("\n");
    }

    return result;
}

} // namespace chess::board
