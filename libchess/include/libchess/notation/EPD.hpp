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
    This file provides functions for working with EPD notation.
    @ingroup notation
 */

#pragma once

#include <expected>
#include <libchess/game/Position.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace chess::notation {

using game::Position;
using std::string;
using std::string_view;

/** A position combined with optional EPD opcodes.

    @ingroup notation
 */
struct [[nodiscard]] EPDPosition final {
    /** The position itself. */
    Position position;

    /** This position's EPD operations.
        Values should not be quoted.
        See the ``epd_ops`` namespace for constants representing names of standard EPD operations.
     */
    std::unordered_map<string, string> operations;

    /** Returns true if the two positions have the same Zobrist hash
        and an identical set of operations.
     */
    [[nodiscard]] auto operator==(const EPDPosition& other) const noexcept -> bool = default;

    /** Refreshes the default move counter operations so that they are in sync
        with the numeric counters in the position field.
     */
    void refresh_default_operations();
};

/** Parses an EPD string.

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup notation
    @relates EPDPosition
    @see parse_all_epds()
 */
[[nodiscard]] auto from_epd(string_view epdString)
    -> std::expected<EPDPosition, string>;

/** Parses all EPDs in a string containing one EPD per line.

    @ingroup notation
    @relates EPDPosition
    @see from_epd()
 */
[[nodiscard]] auto parse_all_epds(string_view fileContent)
    -> std::vector<EPDPosition>;

/** Writes a position to an EPD string.

    @ingroup notation
    @relates EPDPosition
 */
[[nodiscard]] auto to_epd(const EPDPosition& pos) -> string;

/** This namespace contains some constants representing names of standard EPD operations.
    See https://www.chessprogramming.org/Extended_Position_Description.

    @ingroup notation
    @see EPDPosition
 */
namespace epd_ops {
    /** The analysis count depth operation. */
    inline constexpr string_view AnalysisCountDepth { "acd" };

    /** The analysis count nodes operation. */                 // codespell:ignore acn
    inline constexpr string_view AnalysisCountNodes { "acn" }; // codespell:ignore acn

    /** The analysis count seconds operation. */
    inline constexpr string_view AnalysisCountSeconds { "acs" };

    /** The avoid move(s) operation. */
    inline constexpr string_view AvoidMove { "am" };

    /** The best move(s) operation. */
    inline constexpr string_view BestMove { "bm" };

    /** The primary comment operation.
        c1-c9 may also be used.
     */
    inline constexpr string_view Comment { "c0" };

    /** The centipawn evaluation operation. */
    inline constexpr string_view CentipawnEvaluation { "ce" };

    /** The direct mate fullmove count operation. */
    inline constexpr string_view DirectMate { "dm" };

    /** The draw accept operation. */
    inline constexpr string_view DrawAccept { "draw_accept" };

    /** The draw claim operation. */
    inline constexpr string_view DrawClaim { "draw_claim" };

    /** The draw offer operation. */
    inline constexpr string_view DrawOffer { "draw_offer" };

    /** The draw reject operation. */
    inline constexpr string_view DrawReject { "draw_reject" };

    /** The Encyclopedia of Chess Openings opening code operation. */
    inline constexpr string_view EcoCode { "eco" };

    /** The fullmove number operation. */
    inline constexpr string_view FullmoveNumber { "fmvn" };

    /** The halfmove clock operation. */
    inline constexpr string_view HalfmoveClock { "hmvc" };

    /** The position identification operation. */
    inline constexpr string_view PositionID { "id" };

    /** The New In Chess opening code operation. */
    inline constexpr string_view NicCode { "nic" };

    /** The no-op operation. */
    inline constexpr string_view NoOp { "noop" };

    /** The predicted move operation. */
    inline constexpr string_view PredictedMove { "pm" };

    /** The predicted variation operation. */
    inline constexpr string_view PredictedVariation { "pv" };

    /** The repetition count operation. */
    inline constexpr string_view RepetitionCount { "rc" };

    /** The game resignation operation. */
    inline constexpr string_view Resignation { "resign" };

    /** The supplied move operation. */
    inline constexpr string_view SuppliedMove { "sm" };
} // namespace epd_ops

} // namespace chess::notation
