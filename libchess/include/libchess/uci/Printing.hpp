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
    This file provides some utility functions for printing UCI-style @cite Meyer-Kahlen_2006 output.
    @ingroup uci
 */

#pragma once

#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

/** This namespace contains utility functions for printing UCI-style @cite Meyer-Kahlen_2006 output.
    @ingroup uci
 */
namespace chess::uci::printing {

using moves::Move;
using std::size_t;

/** Prints a UCI-formatted @cite Meyer-Kahlen_2006 information string to standard output.
    This function should be used for any informational or debug output that
    an engine wants to print.

    @note This function returns a ``std::monostate`` object so that it is suitable
    for passing to functions such as ``std::expected::transform_error``.

    @ingroup uci
 */
std::monostate info_string(std::string_view info);

/** Prints a UCI-formatted @cite Meyer-Kahlen_2006 best move string to standard output.
    Specifying a ponder move is optional.

    @ingroup uci
 */
void best_move(
    Move                bestMove,
    std::optional<Move> ponderMove);

/** This POD struct encapsulates the various information that can be printed
    about a search.

    @ingroup uci
    @see search_info()

    @todo ``multipv``, ``currmove``, ``currmovenumber``, ``refutation``, ``currline``
 */
struct SearchInfo final {
    /** Represents the engine's evaluation of the line it is currently searching. */
    struct Score {
        /** This type holds a value in internal units (centipawns). */
        struct Centipawns final {
            /** The score in centipawns, from the engine's point of view. */
            int value { 0 };
        };

        /** This struct represents a mate score. */
        struct MateIn final {
            /** The number of plies until mate.
                The value should be negative if the engine is getting mated.
             */
            int plies { 0 };

            /** Returns the number of full moves to mate.
                The value is negative if the engine is getting mated.
             */
            [[nodiscard]] auto moves() const noexcept -> int;
        };

        /** The evaluation value. */
        std::variant<Centipawns, MateIn> value;

        /** True if the score is just a lower bound (ie, a beta cutoff). */
        bool lowerBound { false };

        /** True if the score is just an upper bound (ie, an alpha cutoff). */
        bool upperBound { false };
    };

    /** The engine's evaluation of the root position. */
    Score score;

    /** The search depth, in plies, associated with this information. */
    size_t depth { 0uz };

    /** Selective search depth, in plies. */
    size_t selDepth { 0uz };

    /** The elapsed search time. */
    std::chrono::milliseconds time;

    /** The total number of nodes searched. */
    size_t nodes { 0uz };

    /** In MultiPV mode, this integer should be the number of the line that this info is for.
        In non-MultiPV mode, this should be ``nullopt``.
     */
    std::optional<size_t> multiPV;

    /** The principal variation found. */
    moves::MoveList pv;

    /** The transposition table was filled X permille by this search so far. */
    size_t hashfull { 0uz };

    /** The number of positions found in the endgame tablebases. */
    size_t tbHits { 0uz };

    /** An optional additional information string that will be printed along
        with the rest of the search information.
     */
    std::string extraInformation;

    /** Returns the nodes-per-second for this search. */
    [[nodiscard]] auto get_nps() const noexcept -> size_t;
};

/** Prints a UCI-formatted @cite Meyer-Kahlen_2006 search info string to standard output.

    @ingroup uci
    @relates SearchInfo
 */
void search_info(const SearchInfo& info);

} // namespace chess::uci::printing
