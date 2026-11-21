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
    This file defines the TaperedScore struct.
    @ingroup eval
 */

#pragma once

#include <cstdint>

namespace ben_bot::eval {

/** A pair of middlegame & endgame evaluation values.
    Eval terms return one of these, and ``evaluate()`` blends the total
    by game phase once at the end, using only integer math.

    @ingroup eval
 */
struct TaperedScore final {
    /** The middlegame value, in centipawns. */
    std::int32_t mg { 0 };

    /** The endgame value, in centipawns. */
    std::int32_t eg { 0 };

    /** Returns a score with the same value in the middlegame & endgame. */
    [[nodiscard]] static constexpr auto both(const std::int32_t value) noexcept -> TaperedScore
    {
        return { .mg = value, .eg = value };
    }

    /// @name Arithmetic
    /// @{
    [[nodiscard]] constexpr auto operator+(const TaperedScore& other) const noexcept -> TaperedScore
    {
        return { .mg = mg + other.mg, .eg = eg + other.eg };
    }

    [[nodiscard]] constexpr auto operator-(const TaperedScore& other) const noexcept -> TaperedScore
    {
        return { .mg = mg - other.mg, .eg = eg - other.eg };
    }

    [[nodiscard]] constexpr auto operator-() const noexcept -> TaperedScore
    {
        return { .mg = -mg, .eg = -eg };
    }

    [[nodiscard]] constexpr auto operator*(const int factor) const noexcept -> TaperedScore
    {
        return { .mg = mg * factor, .eg = eg * factor };
    }

    constexpr auto operator+=(const TaperedScore& other) noexcept -> TaperedScore&
    {
        mg += other.mg;
        eg += other.eg;
        return *this;
    }

    constexpr auto operator-=(const TaperedScore& other) noexcept -> TaperedScore&
    {
        mg -= other.mg;
        eg -= other.eg;
        return *this;
    }
    /// @}

    /** Compares two scores for equality. */
    [[nodiscard]] constexpr auto operator==(const TaperedScore&) const noexcept -> bool = default;

    /** Blends the middlegame & endgame values by game phase.
        ``mgPhase`` must be in the range ``[0, maxPhase]``, where ``maxPhase``
        is the pure middlegame. The result is truncated toward zero.
     */
    [[nodiscard]] constexpr auto blend(
        const std::int32_t mgPhase, const std::int32_t maxPhase) const noexcept -> std::int32_t
    {
        return ((mg * mgPhase) + (eg * (maxPhase - mgPhase))) / maxPhase;
    }
};

} // namespace ben_bot::eval
