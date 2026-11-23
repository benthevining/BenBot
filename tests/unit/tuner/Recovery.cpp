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

#include "support/Fixtures.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <tuner/Dataset.hpp>
#include <tuner/Model.hpp>
#include <tuner/Optimiser.hpp>
#include <unordered_set>
#include <utility>
#include <vector>

// Labels a dataset from a known set of weights, then checks that the optimiser,
// starting from the default weights, finds those weights again.

namespace {

using ben_bot::eval::DEFAULT_WEIGHTS;
using ben_bot::eval::offset_of;
using ben_bot::eval::Term;
using ben_bot::tuner::compute_loss;
using ben_bot::tuner::compute_loss_and_gradient;
using ben_bot::tuner::LabelledPosition;
using ben_bot::tuner::Loss;
using ben_bot::tuner::Params;
using std::size_t;

// the fixture's positions, plus every position one move away from them
[[nodiscard]] auto get_positions() -> std::vector<LabelledPosition>
{
    std::vector<LabelledPosition> positions;

    std::unordered_set<chess::game::Position::Hash> seen;

    for (const auto& labelled : tuner_tests::read_positions("positions.epd")) {
        if (seen.insert(labelled.position.hash).second)
            positions.push_back(labelled);

        for (const auto& move : chess::moves::generate(labelled.position)) {
            auto child = after_move(labelled.position, move);

            if (not child.is_check() and seen.insert(child.hash).second)
                positions.push_back({ .position = std::move(child) });
        }
    }

    return positions;
}

struct Change final {
    size_t index { 0 };

    double mg { 0. };

    double eg { 0. };
};

} // namespace

TEST_CASE("Recovery - the optimiser finds the weights that labelled the data", "[tuner][optimiser]")
{
    using chess::pieces::Type;

    // the weights that the data is labelled from: the defaults, with these changed
    const std::array changes {
        Change { .index = offset_of(Term::Material) + static_cast<size_t>(Type::Pawn), .mg = 15., .eg = 30. },
        Change { .index = offset_of(Term::Material) + static_cast<size_t>(Type::Knight), .mg = -25., .eg = -40. },
        Change { .index = offset_of(Term::Material) + static_cast<size_t>(Type::Bishop), .mg = 20., .eg = 10. },
        Change { .index = offset_of(Term::Material) + static_cast<size_t>(Type::Rook), .mg = -30., .eg = 25. },
        Change { .index = offset_of(Term::DoubledPawn), .mg = -12., .eg = -18. },
        Change { .index = offset_of(Term::PawnChain), .mg = 6., .eg = -4. },
        Change { .index = offset_of(Term::CenterControl), .mg = 8., .eg = 0. },
        Change { .index = offset_of(Term::Hanging), .mg = 10., .eg = 20. },
    };

    const auto dataset_positions = get_positions();

    REQUIRE(dataset_positions.size() > 2000uz);

    auto dataset = ben_bot::tuner::build_dataset(dataset_positions, 4uz);

    auto target = ben_bot::tuner::to_params(DEFAULT_WEIGHTS);

    // only the changed weights are tuned, since a few thousand positions can't pin down all of them
    std::vector<bool> mask(target.size(), false);

    for (const auto& change : changes) {
        target.at(change.index).mg += change.mg;
        target.at(change.index).eg += change.eg;

        mask.at(change.index) = true;
    }

    static constexpr auto K = 1.5;

    for (auto& entry : dataset.entries)
        entry.result = static_cast<float>(ben_bot::tuner::sigmoid(ben_bot::tuner::model_eval(dataset, entry, target), K));

    auto params = ben_bot::tuner::to_params(DEFAULT_WEIGHTS);

    const auto startLoss = compute_loss(dataset, params, K, Loss::MeanSquaredError, 4uz);

    REQUIRE(startLoss > 1e-5);

    ben_bot::tuner::Adam adam { mask };

    auto learningRate = 2.;

    for (auto epoch = 1uz; epoch <= 3000uz; ++epoch) {
        const auto [loss, gradient] = compute_loss_and_gradient(dataset, params, K, Loss::MeanSquaredError, 4uz);

        adam.step(params, gradient, learningRate);

        if (epoch % 500uz == 0uz)
            learningRate /= 2.;
    }

    REQUIRE(compute_loss(dataset, params, K, Loss::MeanSquaredError, 4uz) < startLoss * 1e-4);

    for (auto i = 0uz; i < params.size(); ++i) {
        INFO(i);
        INFO(params.at(i).mg << " vs " << target.at(i).mg);
        INFO(params.at(i).eg << " vs " << target.at(i).eg);

        // the weights that weren't tuned don't change
        if (not mask.at(i)) {
            REQUIRE(std::abs(params.at(i).mg - target.at(i).mg) < 1e-12);
            REQUIRE(std::abs(params.at(i).eg - target.at(i).eg) < 1e-12);
            continue;
        }

        REQUIRE(std::abs(params.at(i).mg - target.at(i).mg) < 0.5);
        REQUIRE(std::abs(params.at(i).eg - target.at(i).eg) < 0.5);
    }
}
