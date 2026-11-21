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
    This file defines the search stack.
    @ingroup benbot_data_structures
 */

#pragma once

#include <array>
#include <cassert>
#include <cstddef> // IWYU pragma: keep - for size_t & ptrdiff_t
#include <iterator>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Constants.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>

namespace ben_bot {

using chess::moves::Move;
using std::size_t;

/** Information about one node on the current search path.

    @ingroup benbot_data_structures
    @see SearchStack
 */
struct SearchStackEntry final {
    /** The move that was played to reach this node.
        This is a null move for the child of a null move, and for the
        root & the sentinel entries before it.
     */
    Move move { };

    /** This node's static evaluation, or ``nullopt`` if it doesn't have one.
        Nodes that are in check never have one. Other nodes only have one if
        the search computed it: currently that's non-PV nodes in the main
        search, and quiescence search nodes.
     */
    std::optional<eval::Score> staticEval;
};

/** This structure stores information about each node on the current search path,
    indexed by ply from the root of the search (the root is ply 0).

    There are ``SENTINELS`` entries before the root's entry, which always hold
    a null move and no static eval, so a node can always look at least that many
    plies back without checking how far it is from the root.

    Each node's entry is reset by its parent right before the node is searched,
    so an entry never holds anything left over from an unrelated node that was
    searched earlier at the same ply.

    @ingroup benbot_data_structures
 */
struct SearchStack final {
    /** The number of sentinel entries before the root's entry. */
    static constexpr auto SENTINELS { 4uz };

    /** Returns the entry for the node at the given ply from the root.
        The entries before it, down to ``SENTINELS`` plies before the
        root, can be reached with ``std::prev()``.
     */
    [[nodiscard]] auto at(const size_t plyFromRoot) noexcept -> SearchStackEntry*
    {
        assert(plyFromRoot <= search::MAX_PLY);

        return std::next(entries.data(), static_cast<std::ptrdiff_t>(SENTINELS + plyFromRoot));
    }

    /** Returns the entry for the node at the given ply from the root. */
    [[nodiscard]] auto at(const size_t plyFromRoot) const noexcept -> const SearchStackEntry*
    {
        assert(plyFromRoot <= search::MAX_PLY);

        return std::next(entries.data(), static_cast<std::ptrdiff_t>(SENTINELS + plyFromRoot));
    }

private:
    // the search aborts when it reaches MAX_PLY, so that's the deepest node.
    // The extra entry is for the child entry that a node at MAX_PLY would
    // write if it ever recursed.
    std::array<SearchStackEntry, SENTINELS + search::MAX_PLY + 2uz> entries { };
};

} // namespace ben_bot
