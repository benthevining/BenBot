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
#include <libbenbot/data-structures/OpeningBook.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <span>
#include <vector>

namespace chess::search {

using notation::GameRecord;

void OpeningBook::add_from_pgn(
    const std::string_view pgnText,
    const bool             includeVariations)
{
    for (const auto& game : notation::parse_all_pgns(pgnText))
        add_pgn_moves(game.moves, game.startingPosition, includeVariations);

    prune();
}

void OpeningBook::add_pgn_moves(
    const std::span<const GameRecord::Move> moves,
    Position position, const bool includeVariations)
{
    for (const auto& moveData : moves) {
        lines[position.hash].emplace_back(moveData.move);

        if (includeVariations) {
            for (const auto& variation : moveData.variations)
                add_pgn_moves(variation, position, true);
        }

        position.make_move(moveData.move);
    }
}

void OpeningBook::prune()
{
    for (auto& moves : std::views::values(lines)) {
        std::ranges::sort(moves);

        const auto [first, last] = std::ranges::unique(moves);

        moves.erase(first, last);
    }
}

} // namespace chess::search
