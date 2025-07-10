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
#include <utility>
#include <vector>

namespace chess::search {

using notation::GameRecord;

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

std::vector<GameRecord> OpeningBook::to_pgns() const
{
    std::vector<GameRecord> pgns;

    const Position position;

    for (const auto& rootMove : get_moves(position)) {
        GameRecord game;

        game.moves.emplace_back(rootMove);

        add_next_position(
            after_move(position, rootMove),
            game);

        pgns.emplace_back(std::move(game));
    }

    return pgns;
}

void OpeningBook::add_next_position(
    const Position& position,
    GameRecord&     game) const
{
    const auto moves = get_moves(position);

    if (moves.empty())
        return;

    auto& moveData = game.moves.emplace_back(moves.front());

    for (const auto& move : moves | std::views::drop(1uz)) {
        auto& variation = moveData.variations.emplace_back();

        variation.emplace_back(move);
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
