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

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <libbenbot/eval/Trace.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <limits>
#include <numeric>
#include <string_view>
#include <tuner/Dataset.hpp>
#include <tuner/Model.hpp>
#include <tuner/Parallel.hpp>
#include <utility>
#include <vector>

namespace ben_bot::tuner {

namespace {

    // the loss of one position, and its derivative with respect to the position's eval
    struct PointLoss final {
        double loss { 0. };

        double derivative { 0. };
    };

    [[nodiscard]] auto point_loss(
        const double eval, const double result, const double k, const Loss loss) noexcept -> PointLoss
    {
        const auto expected = sigmoid(eval, k);

        // d(sigmoid)/d(eval) = k/400 * s * (1 - s)
        const auto scale = k / 400.;

        if (loss == Loss::CrossEntropy) {
            // keep log() finite for positions that the model is certain about
            static constexpr auto TINY = 1e-12;

            const auto clamped = std::clamp(expected, TINY, 1. - TINY);

            return {
                .loss       = -((result * std::log(clamped)) + ((1. - result) * std::log(1. - clamped))),
                .derivative = (expected - result) * scale
            };
        }

        const auto error = result - expected;

        return {
            .loss       = error * error,
            .derivative = -2. * error * expected * (1. - expected) * scale
        };
    }

    [[nodiscard]] auto mg_factor(const Entry& entry) noexcept -> double
    {
        return static_cast<double>(entry.phase) / static_cast<double>(eval::MAX_PHASE);
    }

} // namespace

auto to_params(const eval::Weights& weights) -> Params
{
    Params params;

    params.reserve(weights.values.size());

    for (const auto& weight : weights.values)
        params.push_back({ .mg = static_cast<double>(weight.mg), .eg = static_cast<double>(weight.eg) });

    return params;
}

auto round_params(const Params& params) -> eval::Weights
{
    const auto round = [](const double value) {
        static constexpr auto MIN = static_cast<double>(std::numeric_limits<std::int32_t>::min());
        static constexpr auto MAX = static_cast<double>(std::numeric_limits<std::int32_t>::max());

        return static_cast<std::int32_t>(std::clamp(std::round(value), MIN, MAX));
    };

    eval::Weights weights;

    for (auto i = 0uz; i < std::min(params.size(), weights.values.size()); ++i)
        weights.values.at(i) = { .mg = round(params.at(i).mg), .eg = round(params.at(i).eg) };

    return weights;
}

auto recentre(Params params) -> Params
{
    using chess::board::NUM_SQUARES;
    using eval::offset_of;
    using eval::Term;
    using PieceType = chess::pieces::Type;

    // subtracts the average of the weights in [first, last) from each of them, and adds it to
    // the material value of the given piece type (if it has one)
    const auto move_mean_into_material = [&params](const size_t first, const size_t last, const PieceType type) {
        FloatScore mean;

        for (auto i = first; i < last; ++i) {
            mean.mg += params.at(i).mg;
            mean.eg += params.at(i).eg;
        }

        const auto count = static_cast<double>(last - first);

        mean.mg /= count;
        mean.eg /= count;

        for (auto i = first; i < last; ++i) {
            params.at(i).mg -= mean.mg;
            params.at(i).eg -= mean.eg;
        }

        if (type != PieceType::King) {
            auto& material = params.at(offset_of(Term::Material) + static_cast<size_t>(type));

            material.mg += mean.mg;
            material.eg += mean.eg;
        }
    };

    static constexpr auto SQUARES_PER_RANK = 8uz;

    for (const auto type : { PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
             PieceType::Rook, PieceType::Queen, PieceType::King }) {
        const auto table = offset_of(Term::Pst) + (static_cast<size_t>(type) * NUM_SQUARES);

        // pawns are never on the first or last rank
        if (type == PieceType::Pawn)
            move_mean_into_material(table + SQUARES_PER_RANK, table + NUM_SQUARES - SQUARES_PER_RANK, type);
        else
            move_mean_into_material(table, table + NUM_SQUARES, type);
    }

    for (const auto& [term, type] : { std::pair { Term::KnightMobility, PieceType::Knight },
             std::pair { Term::BishopMobility, PieceType::Bishop },
             std::pair { Term::RookMobility, PieceType::Rook },
             std::pair { Term::QueenMobility, PieceType::Queen } }) {
        move_mean_into_material(offset_of(term), offset_of(term) + eval::count_of(term), type);
    }

    return params;
}

auto tunable_mask() -> std::vector<bool>
{
    std::vector<bool> mask;

    mask.reserve(eval::NUM_WEIGHTS);

    for (const auto& term : eval::TERMS)
        mask.insert(mask.end(), term.count, term.tunable);

    return mask;
}

auto parse_loss(const std::string_view text, Loss& loss) -> bool
{
    if (text == "mse") {
        loss = Loss::MeanSquaredError;
        return true;
    }

    if (text == "ce") {
        loss = Loss::CrossEntropy;
        return true;
    }

    return false;
}

auto sigmoid(const double eval, const double k) noexcept -> double
{
    return 1. / (1. + std::exp(-k * eval / 400.));
}

auto model_eval(const Dataset& dataset, const Entry& entry, const Params& params) noexcept -> double
{
    auto middlegame = static_cast<double>(entry.baseMg);
    auto endgame    = static_cast<double>(entry.baseEg);

    for (const auto& tuple : dataset.tuples_of(entry)) {
        const auto& param = params[tuple.index];

        middlegame += param.mg * tuple.coeff;
        endgame += param.eg * tuple.coeff;
    }

    const auto factor = mg_factor(entry);

    return (middlegame * factor) + (endgame * (1. - factor));
}

auto compute_loss(
    const Dataset& dataset, const Params& params, const double k, const Loss loss, const size_t numThreads)
    -> double
{
    if (dataset.empty())
        return 0.;

    std::vector<double> sums(std::max(1uz, numThreads));

    parallel_for(numThreads, dataset.size(), [&](const size_t thread, const size_t begin, const size_t end) {
        auto sum = 0.;

        for (auto i = begin; i < end; ++i) {
            const auto& entry = dataset.entries[i];

            sum += point_loss(model_eval(dataset, entry, params), static_cast<double>(entry.result), k, loss).loss;
        }

        sums.at(thread) = sum;
    });

    // summed in thread order, so that the result doesn't depend on timing
    const auto total = std::accumulate(sums.begin(), sums.end(), 0.);

    return total / static_cast<double>(dataset.size());
}

auto compute_loss_and_gradient(
    const Dataset& dataset, const Params& params, const double k, const Loss loss, const size_t numThreads)
    -> LossAndGradient
{
    LossAndGradient result {
        .gradient = Params(params.size())
    };

    if (dataset.empty())
        return result;

    struct Local final {
        double loss { 0. };

        Params gradient;
    };

    std::vector<Local> locals(std::max(1uz, numThreads));

    parallel_for(numThreads, dataset.size(), [&](const size_t thread, const size_t begin, const size_t end) {
        auto& local = locals.at(thread);

        local.gradient.assign(params.size(), { });

        for (auto i = begin; i < end; ++i) {
            const auto& entry = dataset.entries[i];

            const auto [pointLoss, derivative] = point_loss(model_eval(dataset, entry, params), static_cast<double>(entry.result), k, loss);

            local.loss += pointLoss;

            const auto factor = mg_factor(entry);

            const auto mgDerivative = derivative * factor;
            const auto egDerivative = derivative * (1. - factor);

            for (const auto& tuple : dataset.tuples_of(entry)) {
                auto& gradient = local.gradient[tuple.index];

                gradient.mg += mgDerivative * tuple.coeff;
                gradient.eg += egDerivative * tuple.coeff;
            }
        }
    });

    // reduce in thread order, so that the result doesn't depend on timing
    for (const auto& local : locals) {
        result.loss += local.loss;

        for (auto i = 0uz; i < local.gradient.size(); ++i) {
            result.gradient[i].mg += local.gradient[i].mg;
            result.gradient[i].eg += local.gradient[i].eg;
        }
    }

    const auto count = static_cast<double>(dataset.size());

    result.loss /= count;

    for (auto& gradient : result.gradient) {
        gradient.mg /= count;
        gradient.eg /= count;
    }

    return result;
}

auto add_l2_pull(
    Params& gradient, const Params& params, const Params& start, const double lambda) -> double
{
    const auto mask = tunable_mask();

    auto penalty = 0.;

    for (auto i = 0uz; i < std::min({ gradient.size(), params.size(), start.size(), mask.size() }); ++i) {
        if (not mask.at(i))
            continue;

        const auto mg = params.at(i).mg - start.at(i).mg;
        const auto eg = params.at(i).eg - start.at(i).eg;

        penalty += lambda * ((mg * mg) + (eg * eg));

        gradient.at(i).mg += 2. * lambda * mg;
        gradient.at(i).eg += 2. * lambda * eg;
    }

    return penalty;
}

auto fit_k(
    const Dataset& dataset, const Params& params, const Loss loss, const size_t numThreads,
    double minK, double maxK) -> double
{
    // golden-section search: the loss is unimodal in K
    static const auto INV_PHI = (std::sqrt(5.) - 1.) / 2.;

    static constexpr auto TOLERANCE = 1e-6;

    const auto loss_at = [&](const double k) { return compute_loss(dataset, params, k, loss, numThreads); };

    auto lowK  = maxK - (INV_PHI * (maxK - minK));
    auto highK = minK + (INV_PHI * (maxK - minK));

    auto lowLoss  = loss_at(lowK);
    auto highLoss = loss_at(highK);

    while (maxK - minK > TOLERANCE) {
        if (lowLoss < highLoss) {
            maxK     = highK;
            highK    = lowK;
            highLoss = lowLoss;
            lowK     = maxK - (INV_PHI * (maxK - minK));
            lowLoss  = loss_at(lowK);
        } else {
            minK     = lowK;
            lowK     = highK;
            lowLoss  = highLoss;
            highK    = minK + (INV_PHI * (maxK - minK));
            highLoss = loss_at(highK);
        }
    }

    return std::midpoint(minK, maxK);
}

} // namespace ben_bot::tuner
