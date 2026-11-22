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
    This file defines the tuner's model of the eval, its loss function & gradient,
    and the fit of the sigmoid's scaling constant K.
    @ingroup tuner
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <libbenbot/eval/Weights.hpp>
#include <string_view>
#include <tuner/Dataset.hpp>
#include <vector>

namespace ben_bot::tuner {

/** A middlegame & endgame pair of floating point values.
    @ingroup tuner
 */
struct FloatScore final {
    /** The middlegame value. */
    double mg { 0. };

    /** The endgame value. */
    double eg { 0. };
};

/** The tuner's copy of the eval weights, in the order of ``eval::Weights::values``.
    The engine's weights are integers. The tuner keeps them as floating point numbers
    while it optimises, and only rounds them when it writes them out.

    @ingroup tuner
 */
using Params = std::vector<FloatScore>;

/** Converts the engine's integer weights to tuner parameters.
    @ingroup tuner
 */
[[nodiscard]] auto to_params(const eval::Weights& weights) -> Params;

/** Rounds tuner parameters to the nearest integers, to give the engine's weights.
    @ingroup tuner
 */
[[nodiscard]] auto round_params(const Params& params) -> eval::Weights;

/** Returns, for each weight in ``eval::Weights::values``, whether its term is tunable.
    @ingroup tuner
 */
[[nodiscard]] auto tunable_mask() -> std::vector<bool>;

/** The loss function.
    @ingroup tuner
 */
enum class Loss : std::uint8_t {
    MeanSquaredError, ///< The mean of (result - sigmoid(eval))^2. This is what most tuners use.
    CrossEntropy      ///< The mean of the cross-entropy between the result & sigmoid(eval).
};

/** Parses ``mse`` or ``ce``. Returns false if the string is neither.
    @ingroup tuner
 */
[[nodiscard]] auto parse_loss(std::string_view text, Loss& loss) -> bool;

/** Returns the expected score for an eval in centipawns: ``1 / (1 + exp(-k * eval / 400))``.
    @ingroup tuner
 */
[[nodiscard]] auto sigmoid(double eval, double k) noexcept -> double;

/** Returns the eval of a dataset position with the given parameters, from White's
    point of view. This is the engine's eval without its final truncation to an integer.

    @ingroup tuner
 */
[[nodiscard]] auto model_eval(const Dataset& dataset, const Entry& entry, const Params& params) noexcept
    -> double;

/** Returns the mean loss over the dataset, or 0 if it's empty.
    @ingroup tuner
 */
[[nodiscard]] auto compute_loss(
    const Dataset& dataset, const Params& params, double k, Loss loss, size_t numThreads) -> double;

/** The loss & its gradient with respect to each parameter.
    @ingroup tuner
 */
struct LossAndGradient final {
    /** The mean loss. */
    double loss { 0. };

    /** The partial derivative of ``loss`` with respect to each parameter. */
    Params gradient;
};

/** Returns the mean loss over the dataset and its exact gradient.
    Each thread sums over its own chunk of the dataset into its own gradient, and the
    per-thread results are summed at the end, in thread order.

    @ingroup tuner
 */
[[nodiscard]] auto compute_loss_and_gradient(
    const Dataset& dataset, const Params& params, double k, Loss loss, size_t numThreads)
    -> LossAndGradient;

/** Finds the K that minimises the loss of the given parameters on the dataset,
    using a golden-section search over ``[minK, maxK]``.

    @ingroup tuner
 */
[[nodiscard]] auto fit_k(
    const Dataset& dataset, const Params& params, Loss loss, size_t numThreads,
    double minK = 0.01, double maxK = 10.) -> double;

} // namespace ben_bot::tuner
