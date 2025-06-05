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

#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <libchess/game/Position.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/moves/Move.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace chess::notation {

using game::Position;

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

    /** The starting position of this game. */
    Position startingPosition {};

    /** If the game ended in a conclusive result, this holds the
        appropriate Result enumeration. If the game is ongoing,
        this is ``nullopt``.
     */
    std::optional<game::Result> result;

    /** Records a game move alongside an optional comment and possible variations. */
    struct Move final {
        /** The move. */
        moves::Move move;

        /** The comment string associated with this move.
            Empty if this move has no comment.
         */
        std::string comment;

        /** If this move was annotated with one or more Numerical
            Annotation Glyph, this contains the numerical codes that
            followed each ``$`` character in the original PGN.

            For example, for a move annotated ``!``, this would be 1,
            for a ``?`` this would be 2, etc.
         */
        std::vector<std::uint_least8_t> nags;

        using Variation = std::vector<Move>;

        /** If this move has alternate possible continuations, they are
            stored here. The first move in each of these variations is
            the move that could've been played instead of ``move``.
         */
        std::vector<Variation> variations;
    };

    /** This game's moves. */
    std::vector<Move> moves;

    /** Returns the final position of this game. */
    [[nodiscard]] Position get_final_position() const;
};

/** Parses the text of a PGN file into a GameRecord object.

    @throws std::invalid_argument An exception will be thrown
    if the PGN can not be parsed correctly.

    @ingroup notation
    @relates GameRecord
 */
[[nodiscard]] GameRecord from_pgn(std::string_view pgnText);

/** Creates a PGN string from the given game record.

    @param game The game record to serialize.

    @param useBlockComments If true (the default), move comment strings will
    be written using the ``{<comment>}`` syntax. If false, comments will be
    written using ``; <comment>\n``.

    @ingroup notation
    @relates GameRecord
 */
[[nodiscard]] std::string to_pgn(const GameRecord& game, bool useBlockComments = true);

} // namespace chess::notation
