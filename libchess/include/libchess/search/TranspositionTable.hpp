/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the transposition table data structure.
    @ingroup search
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <libchess/game/Position.hpp>
#include <unordered_map>

namespace chess::search {

using game::Position;
using std::size_t;

/** The transposition table data structure.
    @ingroup search
 */
class TranspositionTable final {
public:
    using HashValue = std::uint64_t;

    /** A record of a previously searched position. */
    struct Record final {
        /** The depth that the position was searched to. */
        size_t searchedDepth { 0uz };

        // TODO: eval
        // node type: PV (exact), cut (min/max)

        // maybe also cache legal moves
    };

    /** Retrieves the stored record for the given position,
        or nullptr if the given position isn't in the table.
     */
    [[nodiscard]] const Record* find(const Position& pos) const
    {
        if (const auto it = records.find(pos.hash);
            it != records.end()) {
            return &it->second;
        }

        return nullptr;
    }

    /** Stores a record for a given position. */
    void store(const Position& pos, const Record& record)
    {
        // TODO: if key was in table, need to determine whether it should be overwritten
        records[pos.hash] = record;
    }

private:
    std::unordered_map<HashValue, Record> records;
};

} // namespace chess::search
