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

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <tuner/Model.hpp>
#include <tuner/Optimiser.hpp>
#include <utility>
#include <vector>

namespace ben_bot::tuner {

Adam::Adam(std::vector<bool> parameterMask)
    : mask { std::move(parameterMask) }
    , firstMoment(mask.size())
    , secondMoment(mask.size())
{
}

void Adam::step(Params& params, const Params& gradient, const double learningRate)
{
    if (params.size() != mask.size() or gradient.size() != mask.size())
        throw std::invalid_argument { "Adam: the parameters, gradient & mask must be the same size" };

    ++steps;

    const auto exponent = static_cast<double>(steps);

    // bias correction: the moments start at zero, so they're too small in the first few steps
    const auto firstCorrection  = 1. - std::pow(BETA1, exponent);
    const auto secondCorrection = 1. - std::pow(BETA2, exponent);

    const auto update = [&](double& param, double& first, double& second, const double grad) {
        first  = (BETA1 * first) + ((1. - BETA1) * grad);
        second = (BETA2 * second) + ((1. - BETA2) * grad * grad);

        const auto meanGrad   = first / firstCorrection;
        const auto meanSquare = second / secondCorrection;

        param -= learningRate * meanGrad / (std::sqrt(meanSquare) + EPSILON);
    };

    for (auto i = 0uz; i < params.size(); ++i) {
        if (not mask[i])
            continue;

        update(params[i].mg, firstMoment[i].mg, secondMoment[i].mg, gradient[i].mg);
        update(params[i].eg, firstMoment[i].eg, secondMoment[i].eg, gradient[i].eg);
    }
}

} // namespace ben_bot::tuner
