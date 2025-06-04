/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides functions for parsing and writing PGN files.
    @ingroup notation
 */

#pragma once

#include <libchess/moves/Move.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace chess::notation {

using moves::Move;

/** A record of a complete game, including some metadata.
    This structure is returned by the ``from_pgn()`` method.

    @ingroup notation
 */
struct GameRecord final {
    /** This game's metadata. The keys will be things like
        "Event", "Site", "Date", etc. The values will not include
        surrounding quotes.
     */
    std::unordered_map<std::string, std::string> metadata;

    /** This game's moves. */
    std::vector<Move> moves;
};

/** Parses the text of a PGN file into a GameRecord object.

    @throws std::invalid_argument An exception will be thrown
    if the PGN can not be parsed correctly.

    @ingroup notation
    @relates GameRecord
 */
[[nodiscard]] GameRecord from_pgn(std::string_view pgnText);

/** Creates a PGN string from the given game record.

    @ingroup notation
    @relates GameRecord
 */
[[nodiscard]] std::string to_pgn(const GameRecord& game);

} // namespace chess::notation
