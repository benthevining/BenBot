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
    This file defines the tuner's optimiser.
    @ingroup tuner
 */

#pragma once

#include <cstddef>
#include <tuner/Model.hpp>
#include <vector>

namespace ben_bot::tuner {

/** Adam with bias correction, over the whole dataset at once.

    The step size of each parameter is about ``learningRate`` centipawns per epoch,
    whatever the scale of its gradient. Parameters that the mask excludes never change.

    @ingroup tuner
 */
class Adam final {
public:
    /** The exponential decay rate of the first moment (the mean of the gradient). */
    static constexpr double BETA1 { 0.9 };

    /** The exponential decay rate of the second moment (the mean of the squared gradient). */
    static constexpr double BETA2 { 0.999 };

    /** Added to the denominator so that a zero gradient doesn't divide by zero. */
    static constexpr double EPSILON { 1e-8 };

    /** Creates an optimiser for ``mask.size()`` parameters.
        Only the parameters whose mask entry is true are updated.
     */
    explicit Adam(std::vector<bool> mask);

    /** Takes one step, moving ``params`` against ``gradient``. */
    void step(Params& params, const Params& gradient, double learningRate);

    /** Returns the number of steps taken so far. */
    [[nodiscard]] auto num_steps() const noexcept -> size_t { return steps; }

private:
    std::vector<bool> mask;

    Params firstMoment;

    Params secondMoment;

    size_t steps { 0uz };
};

} // namespace ben_bot::tuner
