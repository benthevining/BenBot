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
    This file defines the "history gravity" update shared by the history heuristic tables.
    @ingroup benbot_data_structures
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdlib>
#include <limits>
#include <utility>

namespace ben_bot {

/** Applies a bonus (or penalty, if negative) to a history heuristic score.

    This uses the "history gravity" formula: the bonus is first clamped to
    ``[-maxScore, maxScore]``, and then it's scaled down the closer the score
    already is to the limit in the bonus's direction. So a score that starts
    within ``[-maxScore, maxScore]`` always stays within it.

    The maths is done in ``int`` and then narrowed, so this can be used with
    smaller entry types too, as long as they can hold ``maxScore``.

    @ingroup benbot_data_structures
 */
template <std::integral Entry>
void apply_history_gravity(Entry& entry, const int bonus, const int maxScore) noexcept
{
    assert(maxScore > 0);
    assert(std::cmp_less_equal(maxScore, std::numeric_limits<Entry>::max()));

    const auto clamped = std::clamp(bonus, -maxScore, maxScore);

    const int score = entry;

    entry = static_cast<Entry>(score + clamped - (score * std::abs(clamped) / maxScore));
}

} // namespace ben_bot
