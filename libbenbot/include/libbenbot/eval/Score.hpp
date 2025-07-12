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
#include <libbenbot/data-structures/TranspositionTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>

namespace chess::eval {

using std::size_t;

/** An evaluation score.
    This is essentially a wrapper around an integer value, with a few helper
    methods.

    @ingroup eval
 */
struct Score final {
    /** The evaluaton value, in centipawns. */
    int value { 0 };

    /** Implicitly converts this score object to its integer value.
        This method is intentionally not explicit, which allows score
        objects to be transparently compared as integers.
     */
    constexpr operator int() const noexcept { return value; } // NOLINT;

    /** Inverts the score. */
    [[nodiscard]] Score operator-() const noexcept { return { -value }; }

    /// @name Mate queries
    /// @{

    /** Returns true if this score represents checkmate (either winning or losing). */
    [[nodiscard]] constexpr bool is_mate() const noexcept { return std::abs(value) >= MATE; }

    /** Returns true if this score is a winning mate score. */
    [[nodiscard]] constexpr bool is_winning_mate() const noexcept { return value >= MATE; }

    /** Returns true if this score is a losing mate score. */
    [[nodiscard]] constexpr bool is_losing_mate() const noexcept { return value <= -MATE; }

    /** For a checkmate score, returns the number of plies from the root of the search
        tree to the checkmate position. This method asserts if the score is not mate.
     */
    [[nodiscard]] constexpr size_t ply_to_mate() const noexcept
    {
        assert(is_mate());

        return static_cast<size_t>(MAX - std::abs(value));
    }

    /// @}

    /** Returns a value suitable for storing in the transposition table.
        During search, mate scores are based on ply from the root position;
        this function maps all mate scores to the MATE constant.
     */
    [[nodiscard]] constexpr int to_tt() const noexcept;

    /** Returns a checkmate score.
        During search, mate scores are based on the distance from the root of the tree,
        so that the engine actually goes for mate.
     */
    [[nodiscard, gnu::const]] static constexpr Score mate(const size_t plyFromRoot) noexcept
    {
        // multiply by -1 here because this score is relative to the player who got mated
        return { (MAX - static_cast<int>(plyFromRoot)) * -1 };
    }

    using TT = search::TranspositionTable;

    /** Converts a value from the transposition table to a score.
        This maps the MATE constant to a ply-from-root mate score.
     */
    [[nodiscard, gnu::const]] static constexpr Score from_tt(
        const TT::ProbedEval& eval,
        size_t                plyFromRoot) noexcept;
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

constexpr int Score::to_tt() const noexcept
{
    if (is_losing_mate())
        return -MATE;

    if (is_winning_mate())
        return MATE;

    return value;
}

constexpr Score Score::from_tt(
    const TT::ProbedEval& eval,
    const size_t          plyFromRoot) noexcept
{
    const auto [score, type] = eval;

    if (type == TT::Record::EvalType::Exact) {
        if (score <= -MATE)
            return mate(plyFromRoot);

        if (score >= MATE)
            return -mate(plyFromRoot);
    }

    return { score };
}

} // namespace chess::eval
