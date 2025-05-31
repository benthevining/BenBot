/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/UTF8.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace chess::game {

bool Position::is_checkmate() const
{
    return is_check()
        && ! moves::any_legal_moves(*this);
}

bool Position::is_stalemate() const
{
    return ! is_check()
        && ! moves::any_legal_moves(*this);
}

bool Position::is_fifty_move_draw() const
{
    return std::cmp_greater_equal(halfmoveClock, 100)
        && moves::any_legal_moves(*this);
}

namespace utf8_pieces = pieces::utf8;

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

static constexpr std::string_view separator { "|" };

static constexpr std::string_view emptySquare { " |" };

std::string print_utf8(const Position& position)
{
    using board::Rank;
    using board::Square;

    const auto& whitePieces = position.whitePieces;
    const auto& blackPieces = position.blackPieces;

    std::string result;

    result.reserve(208uz);

    for (const auto rank : std::views::reverse(magic_enum::enum_values<Rank>())) {
        result.append(separator);

        for (const auto file : magic_enum::enum_values<File>()) {
            const Square square { file, rank };

            if (const auto piece = whitePieces.get_piece_on(square)) {
                result.append(utf8_pieces::white::get(*piece));
                result.append(separator);
                continue;
            }

            if (const auto piece = blackPieces.get_piece_on(square)) {
                result.append(utf8_pieces::black::get(*piece));
                result.append(separator);
            } else {
                result.append(emptySquare);
            }
        }

        result.append("\n");
    }

    return result;
}

} // namespace chess::game
