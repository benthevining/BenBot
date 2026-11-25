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
#include <initializer_list>
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
        Nodes that are in check never have one. Every other node in the main
        search has one (PV & non-PV nodes alike), as do the root and quiescence
        search nodes. The sentinel entries before the root never have one.
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

/** Returns true if the given node's static eval is better than it was at the same side's
    previous turn, i.e. the node 2 plies earlier on the search path.

    - If this node has no static eval (because it's in check), it's never improving.
    - If the node 2 plies earlier has no static eval (it was in check, or it's a sentinel
      before the root), this compares with the node 4 plies earlier instead.
    - If neither of those has a static eval, there's nothing to compare with, and this
      returns true. That makes pruning that depends on this flag more cautious when we know
      the least.

    The comparison is strict, so a node whose eval hasn't changed isn't improving.

    The entry must be in a SearchStack at or after the root, so that the 4 entries before
    it are valid (they may be sentinels).

    @ingroup benbot_data_structures
    @relates SearchStack
 */
[[nodiscard]] inline auto is_improving(const SearchStackEntry* const entry) noexcept -> bool
{
    assert(entry != nullptr);

    const auto current = entry->staticEval;

    if (not current.has_value())
        return false;

    static_assert(SearchStack::SENTINELS >= 4uz);

    for (const auto pliesBack : { 2, 4 }) {
        if (const auto previous = std::prev(entry, pliesBack)->staticEval)
            return *current > *previous;
    }

    return true;
}

} // namespace ben_bot
