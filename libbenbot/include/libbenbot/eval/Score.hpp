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
    This file defines the Score struct.
    @ingroup eval
 */

#pragma once

#include <cassert>
#include <cmath>   // IWYU pragma: keep - for std::abs()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint>
#include <format>
#include <libchess/uci/Printing.hpp>
#include <limits>

namespace ben_bot::eval {

using std::size_t;

/** Signed integer type used to represent evaluation values.
    @ingroup eval
 */
using Value = std::int16_t;

/** Arbitrary value used as the starting beta value for alpha/beta search.
    This should be larger than mate, but smaller than the data type's max
    (to avoid issues with sign flipping).

    @ingroup eval
 */
static constexpr Value MAX { std::numeric_limits<Value>::max() - 5 };

/** The maximum possible evaluation score, i.e., if the side to move
    has mate-in-1. If the side to move is in checkmate, the evaluation
    is ``-MATE``.

    @ingroup eval
 */
static constexpr Value MATE { MAX / 2 };

/** A neutral, or draw, score.
    @ingroup eval
 */
static constexpr Value DRAW { 0 };

/** An evaluation score.
    This is essentially a wrapper around an integer value, with a few helper
    methods.

    @ingroup eval
 */
struct Score final {
    /** The evaluation value, in centipawns. */
    Value value { 0 };

    /** Implicitly converts this score object to its integer value.
        This method is intentionally not explicit, which allows score
        objects to be transparently compared as integers.
     */
    constexpr operator Value() const noexcept { return value; } // NOLINT;

    /** Inverts the score. */
    [[nodiscard]] constexpr auto operator-() const noexcept -> Score { return { static_cast<Value>(-value) }; }

    /// @name Mate queries
    /// @{

    /** Returns true if this score represents checkmate (either winning or losing). */
    [[nodiscard]] auto is_mate() const noexcept -> bool { return std::abs(value) >= MATE; }

    /** Returns true if this score is a winning mate score. */
    [[nodiscard]] constexpr auto is_winning_mate() const noexcept -> bool { return value >= MATE; }

    /** Returns true if this score is a losing mate score. */
    [[nodiscard]] constexpr auto is_losing_mate() const noexcept -> bool { return value <= -MATE; }

    /** For a checkmate score, returns the number of plies from the root of the search
        tree to the checkmate position. This method asserts if the score is not mate.
     */
    [[nodiscard]] auto ply_to_mate() const noexcept -> size_t
    {
        assert(is_mate());

        return static_cast<size_t>(MAX - std::abs(value));
    }

    /// @}

    /** Returns a value suitable for storing in the transposition table.
        During search, mate scores are based on ply from the root position;
        this function maps all mate scores to the MATE constant.
     */
    [[nodiscard]] constexpr auto to_tt() const noexcept -> Value;

    /** The libchess type used for printing UCI-formatted search information. */
    using LibchessScore = chess::uci::printing::SearchInfo::Score;

    /** Converts this score object to the libchess type used for printing UCI info. */
    [[nodiscard]] auto to_libchess() const noexcept -> LibchessScore;

    /** Returns a checkmate score.
        During search, mate scores are based on the distance from the root of the tree,
        so that the engine actually goes for mate.
     */
    [[nodiscard, gnu::const]] static constexpr auto mate(const size_t plyFromRoot) noexcept -> Score
    {
        // multiply by -1 here because this score is relative to the player who got mated
        return { static_cast<Value>(
            (MAX - static_cast<Value>(plyFromRoot)) * -1) };
    }

    /** Converts a value from the transposition table to a score.
        This maps the MATE constant to a ply-from-root mate score.
     */
    [[nodiscard, gnu::const]] static constexpr auto from_tt(
        Value eval, size_t plyFromRoot) noexcept
        -> Score;
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

constexpr auto Score::to_tt() const noexcept -> Value
{
    if (is_losing_mate())
        return -MATE;

    if (is_winning_mate())
        return MATE;

    return value;
}

constexpr auto Score::from_tt(
    const Value eval, const size_t plyFromRoot) noexcept
    -> Score
{
    if (eval <= -MATE)
        return mate(plyFromRoot);

    if (eval >= MATE)
        return -mate(plyFromRoot);

    return { eval };
}

inline auto Score::to_libchess() const noexcept -> LibchessScore
{
    LibchessScore res;

    if (is_mate()) {
        auto ply = static_cast<int>(ply_to_mate());

        if (is_losing_mate())
            ply *= -1;

        res.mate = ply;
    } else {
        res.cp = value;
    }

    return res;
}

} // namespace ben_bot::eval

/** A specialization of ``std::formatter`` for Score objects.
    The formatter accepts no format arguments, and simply prints
    the score's integer value.

    @see ben_bot::eval::Score
    @ingroup eval
 */
template <>
struct std::formatter<ben_bot::eval::Score> final {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(
        const ben_bot::eval::Score& score, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(), "{}", score.value);
    }
};
