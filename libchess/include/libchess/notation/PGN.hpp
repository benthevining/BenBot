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
    This file provides functions for parsing and writing PGN @cite Edwards_1994 files.
    @ingroup notation
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <expected>
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
using std::string;

/** PGN @cite Edwards_1994 NAG glyphs are represented by their numeric codes.
    This enumeration lists many of the standard NAGs (but not all of them).

    @ingroup notation
 */
enum class NAG : std::uint_least8_t {
    Null                   = 0,  ///< A null annotation. Provided for usage as a placeholder value; should not appear in PGN files and has no typographic representation.
    Good                   = 1,  ///< A good move, typically displayed as ``!``.
    Brilliant              = 3,  ///< A brilliant move, typically displayed as ``!!``.
    Inaccuracy             = 2,  ///< A mistake, typically displayed as ``?``.
    Blunder                = 4,  ///< A blunder, typically displayed as ``??``.
    Interesting            = 5,  ///< A speculative or interesting move, typically displayed as ``!?``.
    Dubious                = 6,  ///< A questionable or dubious move, typically displayed as ``?!``.
    Forced                 = 7,  ///< A forced or only move, typically displayed as a white square.
    Drawish                = 10, ///< Indicates that the position is drawish or double-sided, typically displayed as ``=``.
    Unclear                = 13, ///< Indicates that the position is unclear, typically displayed as an infinity symbol.
    WhiteSlightAdvantage   = 14, ///< Indicates that white has a slight advantage in this position.
    WhiteModerateAdvantage = 16, ///< Indicates that white has a moderate advantage in this position.
    WhiteDecisiveAdvantage = 18, ///< Indicates that white has a decisive advantage in this position.
    WhiteCrushingAdvantage = 20, ///< Indicates that white has a crushing advantage in this position (black should resign).
    BlackSlightAdvantage   = 15, ///< Indicates that black has a slight advantage in this position.
    BlackModerateAdvantage = 17, ///< Indicates that black has a moderate advantage in this position.
    BlackDecisiveAdvantage = 19, ///< Indicates that black has a decisive advantage in this position.
    BlackCrushingAdvantage = 21, ///< Indicates that black has a crushing advantage in this position (white should resign).
    WhiteZugzwang          = 22, ///< Indicates that white is in Zugzwang in this position.
    WhiteInitiative        = 36, ///< Indicates that white has the initiative in this position.
    BlackInitiative        = 37  ///< Indicates that black has the initiative in this position.
};

/** A record of a complete game, including some metadata.
    This structure is returned by the ``from_pgn()`` method.

    @ingroup notation
 */
struct [[nodiscard]] GameRecord final {
    /** This game's metadata. The keys will be things like
        "Event", "Site", "Date", etc. The values will not include
        surrounding quotes.
     */
    std::unordered_map<string, string> metadata;

    /** The starting position of this game. */
    Position startingPosition;

    /** If the game ended in a conclusive result, this holds the
        appropriate Result enumeration. If the game is ongoing,
        this is ``nullopt``.

        Note that this may be different from ``record.get_final_position().get_result()``,
        as that function only accounts for decisive board state. This value
        may be set if a player resigned or the game was adjudicated.
     */
    std::optional<game::Result> result;

    /** Records a game move alongside an optional comment and possible variations. */
    struct Move final {
        /** The move. */
        moves::Move move;

        /** The comment string associated with this move.
            Empty if this move has no comment.
         */
        string comment;

        /** If this move was annotated with one or more Numerical
            Annotation Glyph, this contains the numerical codes that
            followed each ``$`` character in the original PGN.

            For example, for a move annotated ``!``, this would be 1,
            for a ``?`` this would be 2, etc.

            See the ``nags`` namespace for some useful NAG constants.
         */
        std::vector<NAG> nags;

        /** A variation is simply a nested list of moves. */
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
    [[nodiscard]] auto get_final_position() const -> Position;
};

/** Parses the text of a PGN @cite Edwards_1994 file into a GameRecord object.

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup notation
    @relates GameRecord
    @see parse_all_pgns()
 */
[[nodiscard]] auto from_pgn(std::string_view pgnText)
    -> std::expected<GameRecord, std::string_view>;

/** Parses a text file that may contain 0 or more PGNs into a list of
    GameRecord objects. PGNs in the ``fileContent`` should be separated
    by at least 1 newline character. If parsing any of the PGNs fails,
    they will simply be omitted from the list, and any exceptions raised
    by parsing will not be propagated.

    @ingroup notation
    @relates GameRecord
    @see from_pgn()
 */
[[nodiscard]] auto parse_all_pgns(std::string_view fileContent)
    -> std::vector<GameRecord>;

/** Creates a PGN @cite Edwards_1994 string from the given game record.

    @param game The game record to serialize.

    @param useBlockComments If true (the default), move comment strings will
    be written using the ``{<comment>}`` syntax. If false, comments will be
    written using ``; <comment>\n``.

    @ingroup notation
    @relates GameRecord
 */
[[nodiscard]] auto to_pgn(const GameRecord& game, bool useBlockComments = true) -> string;

} // namespace chess::notation
