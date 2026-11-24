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
    This file defines the eval tracers, which record how many times each eval weight
    was applied, so that the eval can be rebuilt from the weights without the board.
    @ingroup eval
 */

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/pieces/Colors.hpp>
#include <utility>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace ben_bot::eval {

using chess::game::Position;
using chess::pieces::Color;

/** How much each non-pawn piece counts towards the game phase.
    This is Fruit's phase: pawns & kings don't count.

    @ingroup eval
 */
namespace phase_weights {

    /// @ingroup eval
    /// @{

    /** The phase weight of a knight. */
    inline constexpr std::int32_t KNIGHT { 1 };

    /** The phase weight of a bishop. */
    inline constexpr std::int32_t BISHOP { 1 };

    /** The phase weight of a rook. */
    inline constexpr std::int32_t ROOK { 2 };

    /** The phase weight of a queen. */
    inline constexpr std::int32_t QUEEN { 4 };

    /// @}

} // namespace phase_weights

/** The game phase of a pure middlegame.
    The eval blends its middlegame & endgame scores with a phase in ``[0, MAX_PHASE]``,
    which is the sum of the phase weights of the pieces on the board, capped at this
    value (which is the phase of the starting position), so that promotions don't push
    it past a pure middlegame.

    @ingroup eval
 */
inline constexpr std::int32_t MAX_PHASE
    = (phase_weights::QUEEN + (phase_weights::ROOK * 2) + (phase_weights::BISHOP * 2) + (phase_weights::KNIGHT * 2)) * 2;

static_assert(MAX_PHASE == 24);

/** The tracer that the engine's eval uses. It records nothing, and compiles away.
    @ingroup eval
 */
struct NullTracer final {
    /** True if this tracer records anything. */
    static constexpr bool ENABLED = false;

    /** Called each time a term applies ``count`` times for ``side``. */
    static constexpr void add(
        [[maybe_unused]] const Term term, [[maybe_unused]] const Color side,
        [[maybe_unused]] const int count, [[maybe_unused]] const size_t index = 0uz) noexcept
    {
    }

    /** Called with the parts of the eval that aren't traced by weight, from ``side``'s point of view. */
    static constexpr void add_base(
        [[maybe_unused]] const Color side, [[maybe_unused]] const TaperedScore& part) noexcept
    {
    }
};

/** A tracer that records, for each eval weight, how many times it applied for each side.

    The eval is then ``score(DEFAULT_WEIGHTS)``: the sum of each weight times its White
    coefficient minus its Black coefficient, plus ``base``, blended by ``phase`` and negated
    if Black is to move. A tuner can call ``score()`` with other weights.

    @ingroup eval
    @see evaluate_traced()
 */
struct CoeffTracer final {
    /** True if this tracer records anything. */
    static constexpr bool ENABLED = true;

    /** The number of times each weight applied for each side, indexed by ``[weight][Color]``.
        The weights are in the order of ``Weights::values``. These can be negative, for terms
        such as space that count our squares minus theirs.
     */
    std::array<std::array<std::int16_t, 2uz>, NUM_WEIGHTS> coeffs { };

    /** The parts of the eval that aren't traced by weight, from White's point of view.
        These are the endgame conversion helpers (the mop-up & last piece terms), which
        aren't tunable and only apply behind gates on the fixed ``piece_values``.
     */
    TaperedScore base { };

    /** The game phase used to blend the middlegame & endgame scores, in ``[0, MAX_PHASE]``. */
    std::int32_t phase { MAX_PHASE };

    /** The side to move. The eval is from this side's point of view. */
    Color sideToMove { Color::White };

    /** Records that a term applied ``count`` times for ``side``. */
    constexpr void add(
        const Term term, const Color side, const int count, const size_t index = 0uz) noexcept
    {
        assert(index < count_of(term));

        auto& coeff = coeffs[offset_of(term) + index][std::to_underlying(side)];

        coeff = static_cast<std::int16_t>(coeff + count);
    }

    /** Adds a part of the eval that isn't traced by weight, from ``side``'s point of view. */
    constexpr void add_base(const Color side, const TaperedScore& part) noexcept
    {
        if (side == Color::White)
            base += part;
        else
            base -= part;
    }

    /** Returns the White coefficient minus the Black coefficient of the weight at ``index``
        in ``Weights::values``.
     */
    [[nodiscard]] constexpr auto coefficient(const size_t index) const noexcept -> int
    {
        const auto& coeff = coeffs[index];

        return coeff[std::to_underlying(Color::White)] - coeff[std::to_underlying(Color::Black)];
    }

    /** Rebuilds the eval's middlegame & endgame scores from the given weights,
        from White's point of view.
     */
    [[nodiscard]] constexpr auto white_score(const Weights& weights) const noexcept -> TaperedScore
    {
        auto total = base;

        for (auto i = 0uz; i < NUM_WEIGHTS; ++i)
            total += weights.values[i] * coefficient(i);

        return total;
    }

    /** Rebuilds the eval from the given weights, from the side to move's point of view.
        With ``DEFAULT_WEIGHTS``, this is exactly what ``evaluate()`` returns.
     */
    [[nodiscard]] constexpr auto score(const Weights& weights) const noexcept -> Score
    {
        const auto white = white_score(weights).blend(phase, MAX_PHASE);

        return Score { static_cast<Value>(sideToMove == Color::White ? white : -white) };
    }
};

/** Evaluates the given position, and returns the trace of the eval.
    The traced eval never uses a pawn table.

    @ingroup eval
    @see CoeffTracer
 */
[[nodiscard]] auto evaluate_traced(const Position& position) -> CoeffTracer;

} // namespace ben_bot::eval
