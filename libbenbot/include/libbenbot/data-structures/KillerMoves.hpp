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

/** @file
    This file defines the killer moves data structure.
    @ingroup benbot_data_structures
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/search/Constants.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <span>
#include <vector>

namespace ben_bot {

using chess::moves::Move;
using std::size_t;

/** This structure stores killer moves collected during the search.
    This is essentially a set of lists of moves, indexed by ply from
    the root of the search.

    @ingroup benbot_data_structures
 */
struct KillerMoves final {
    /** Clears all the stored moves. */
    void clear()
    {
        for (auto& list : lists)
            list.clear();
    }

    /** Stores a killer move. */
    void store(const size_t plyFromRoot, const Move move)
    {
        auto& list = lists[plyFromRoot];

        if (not std::ranges::contains(list, move))
            list.emplace_back(move);
    }

    /** Returns the killer moves for the given ply. */
    [[nodiscard]] auto get(const size_t plyFromRoot) const noexcept -> std::span<const Move>
    {
        return lists[plyFromRoot];
    }

private:
    using Killers = std::vector<Move>;

    std::array<Killers, search::MAX_PLY> lists {};
};

} // namespace ben_bot
