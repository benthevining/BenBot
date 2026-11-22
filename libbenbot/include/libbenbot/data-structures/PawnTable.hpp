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
    This file defines the pawn hash table data structure.
    @ingroup benbot_data_structures
 */

#pragma once

#include <algorithm>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint>
#include <libchess/board/Bitboard.hpp>
#include <vector>

namespace ben_bot {

/** An entry in the pawn hash table.
    This holds the parts of the pawn structure evaluation that only depend on
    the positions of the pawns.

    @ingroup benbot_data_structures
 */
struct PawnEntry final {
    /** The full pawn hash of the position this entry was stored for. */
    std::uint64_t key { UINT64_C(0) };

    /** Both sides' passed pawns. */
    chess::board::Bitboard passers;

    /** The sum of all the pawn-only evaluation terms, from White's point of view. */
    std::int32_t score { 0 };
};

/** A fixed-size cache of pawn structure evaluations, indexed by the
    position's pawn hash. New entries always replace old ones.
    Each search thread owns its own table, so no locking is needed.

    @ingroup benbot_data_structures
 */
class PawnTable final {
public:
    /** The number of entries in the table. This must be a power of 2. */
    static constexpr auto NumEntries { std::size_t { 1 } << 14uz };

    static_assert((NumEntries & (NumEntries - 1uz)) == 0uz);

    /** Returns the entry slot for the given pawn hash.
        The caller must check the entry's ``key`` to see if it's a hit.
     */
    [[nodiscard]] auto probe(const std::uint64_t key) noexcept -> PawnEntry&
    {
        return entries[static_cast<std::size_t>(key & (NumEntries - 1uz))];
    }

    /** Clears all entries in the table. */
    void clear() noexcept
    {
        std::ranges::fill(entries, PawnEntry { });
    }

private:
    std::vector<PawnEntry> entries = std::vector<PawnEntry>(NumEntries);
};

} // namespace ben_bot
