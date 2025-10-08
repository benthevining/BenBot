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

#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/search/Constants.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <span>

namespace ben_bot {

using chess::moves::Move;
using chess::moves::MoveList;
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
    void store(const size_t plyFromRoot, const Move move) noexcept
    {
        lists[plyFromRoot].emplace_back(move);
    }

    /** Returns the killer moves for the given ply. */
    [[nodiscard]] auto get(const size_t plyFromRoot) const noexcept -> std::span<const Move>
    {
        return lists[plyFromRoot];
    }

private:
    std::array<MoveList, search::MAX_PLY> lists {};
};

} // namespace ben_bot
