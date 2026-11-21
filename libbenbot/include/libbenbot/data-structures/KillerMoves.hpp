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
#include <cassert>
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
    the root of the search. Each list holds at most ``MAX_KILLERS``
    moves, with the most recently stored move first.

    @ingroup benbot_data_structures
 */
struct KillerMoves final {
    /** The maximum number of killer moves stored for each ply. */
    static constexpr auto MAX_KILLERS { 2uz };

    /** Clears all the stored moves. */
    void clear()
    {
        for (auto& list : lists)
            list.clear();
    }

    /** Stores a killer move.
        If the list for this ply is full, the oldest move is removed.
     */
    void store(const size_t plyFromRoot, const Move move)
    {
        assert(plyFromRoot < search::MAX_PLY);

        auto& list = lists.at(plyFromRoot);

        if (std::ranges::contains(list, move))
            return;

        list.insert(list.begin(), move);

        if (list.size() > MAX_KILLERS)
            list.pop_back();
    }

    /** Returns the killer moves for the given ply. */
    [[nodiscard]] auto get(const size_t plyFromRoot) const noexcept -> std::span<const Move>
    {
        return lists.at(plyFromRoot);
    }

private:
    using Killers = std::vector<Move>;

    std::array<Killers, search::MAX_PLY + 1uz> lists { };
};

} // namespace ben_bot
