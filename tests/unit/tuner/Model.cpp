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
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Trace.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <string>
#include <tuner/Dataset.hpp>
#include <tuner/Model.hpp>
#include <tuner/Writer.hpp>
#include <utility>
#include <vector>

namespace {

using ben_bot::eval::count_of;
using ben_bot::eval::DEFAULT_WEIGHTS;
using ben_bot::eval::NUM_WEIGHTS;
using ben_bot::tuner::compute_loss;
using ben_bot::tuner::compute_loss_and_gradient;
using ben_bot::tuner::Loss;
using ben_bot::tuner::Params;
using ben_bot::tuner::to_params;

constexpr auto TAGS = "[tuner][model]";

// the defaults, with every tunable weight moved by a different few centipawns
[[nodiscard]] auto perturbed_params() -> Params
{
    auto params = to_params(DEFAULT_WEIGHTS);

    const auto mask = ben_bot::tuner::tunable_mask();

    for (auto i = 0uz; i < params.size(); ++i) {
        if (not mask.at(i))
            continue;

        params.at(i).mg += static_cast<double>(static_cast<int>(i % 7uz) - 3) * 1.5;
        params.at(i).eg += static_cast<double>(static_cast<int>(i % 5uz) - 2) * 2.5;
    }

    return params;
}

void check_gradient(const Loss loss)
{
    const auto dataset = ben_bot::tuner::build_dataset(tuner_tests::read_positions("positions.epd"), 1uz);

    static constexpr auto K = 1.6;

    auto params = perturbed_params();

    const auto [baseLoss, gradient] = compute_loss_and_gradient(dataset, params, K, loss, 1uz);

    REQUIRE(std::abs(baseLoss - compute_loss(dataset, params, K, loss, 1uz)) < 1e-15);

    // the finite difference step, in centipawns
    static constexpr auto STEP = 0.1;

    // a five-point central difference: its truncation error is O(STEP^4), so the step can be
    // large enough that the rounding error in the losses stays well below the tolerance
    const auto numeric_derivative = [&](double& param) {
        const auto original = param;

        const auto loss_at = [&](const double offset) {
            param = original + offset;
            return compute_loss(dataset, params, K, loss, 1uz);
        };

        const auto above  = loss_at(STEP);
        const auto below  = loss_at(-STEP);
        const auto above2 = loss_at(2. * STEP);
        const auto below2 = loss_at(-2. * STEP);

        param = original;

        return ((8. * (above - below)) - (above2 - below2)) / (12. * STEP);
    };

    const auto matches = [](const double analytic, const double numeric) {
        return std::abs(analytic - numeric) <= (1e-6 * std::max(std::abs(analytic), std::abs(numeric))) + 1e-12;
    };

    auto numNonZero = 0uz;

    for (auto i = 0uz; i < NUM_WEIGHTS; ++i) {
        INFO(i);

        const auto mgNumeric = numeric_derivative(params.at(i).mg);
        const auto egNumeric = numeric_derivative(params.at(i).eg);

        INFO(gradient.at(i).mg << " vs " << mgNumeric);
        REQUIRE(matches(gradient.at(i).mg, mgNumeric));

        INFO(gradient.at(i).eg << " vs " << egNumeric);
        REQUIRE(matches(gradient.at(i).eg, egNumeric));

        if (gradient.at(i).mg != 0. or gradient.at(i).eg != 0.)
            ++numNonZero;
    }

    // so that the test isn't vacuous
    REQUIRE(numNonZero > 200uz);
}

} // namespace

TEST_CASE("Model - the gradient matches finite differences", TAGS)
{
    SECTION("Mean squared error")
    {
        check_gradient(Loss::MeanSquaredError);
    }

    SECTION("Cross-entropy")
    {
        check_gradient(Loss::CrossEntropy);
    }
}

TEST_CASE("Model - the result doesn't depend on the number of threads", TAGS)
{
    const auto dataset = ben_bot::tuner::build_dataset(tuner_tests::read_positions("positions.epd"), 2uz);

    const auto params = perturbed_params();

    const auto single = compute_loss_and_gradient(dataset, params, 1.6, Loss::MeanSquaredError, 1uz);

    for (const auto threads : { 2uz, 3uz, 8uz, 200uz }) {
        INFO(threads);

        const auto multi = compute_loss_and_gradient(dataset, params, 1.6, Loss::MeanSquaredError, threads);

        REQUIRE(std::abs(multi.loss - single.loss) < 1e-12);
        REQUIRE(std::abs(compute_loss(dataset, params, 1.6, Loss::MeanSquaredError, threads) - single.loss) < 1e-12);

        for (auto i = 0uz; i < NUM_WEIGHTS; ++i) {
            REQUIRE(std::abs(multi.gradient.at(i).mg - single.gradient.at(i).mg) < 1e-12);
            REQUIRE(std::abs(multi.gradient.at(i).eg - single.gradient.at(i).eg) < 1e-12);
        }
    }
}

TEST_CASE("Model - non-tunable weights have no gradient", TAGS)
{
    const auto dataset = ben_bot::tuner::build_dataset(tuner_tests::read_positions("positions.epd"), 1uz);

    const auto [loss, gradient] = compute_loss_and_gradient(dataset, perturbed_params(), 1.6, Loss::MeanSquaredError, 1uz);

    const auto mask = ben_bot::tuner::tunable_mask();

    REQUIRE(mask.size() == NUM_WEIGHTS);

    for (auto i = 0uz; i < NUM_WEIGHTS; ++i) {
        if (mask.at(i))
            continue;

        REQUIRE(gradient.at(i).mg == 0.);
        REQUIRE(gradient.at(i).eg == 0.);
    }
}

TEST_CASE("Model - K fit", TAGS)
{
    const auto dataset = ben_bot::tuner::build_dataset(tuner_tests::read_positions("positions.epd"), 1uz);

    const auto params = to_params(DEFAULT_WEIGHTS);

    const auto k = ben_bot::tuner::fit_k(dataset, params, Loss::MeanSquaredError, 2uz);

    REQUIRE(k > 0.01);
    REQUIRE(k < 10.);

    const auto loss_at = [&](const double value) {
        return compute_loss(dataset, params, value, Loss::MeanSquaredError, 1uz);
    };

    // K is a minimum of the loss
    REQUIRE(loss_at(k) <= loss_at(k * 1.01));
    REQUIRE(loss_at(k) <= loss_at(k / 1.01));
}

TEST_CASE("Model - sigmoid", TAGS)
{
    using ben_bot::tuner::sigmoid;

    REQUIRE(sigmoid(0., 1.5) == 0.5);
    REQUIRE(std::abs(sigmoid(100., 1.5) + sigmoid(-100., 1.5) - 1.) < 1e-15);
    REQUIRE(std::abs(sigmoid(400., 1.) - (1. / (1. + std::exp(-1.)))) < 1e-15);
}

TEST_CASE("Model - rounding", TAGS)
{
    using ben_bot::tuner::round_params;

    REQUIRE(round_params(to_params(DEFAULT_WEIGHTS)) == DEFAULT_WEIGHTS);

    auto params = to_params(DEFAULT_WEIGHTS);

    params.at(0uz) = { .mg = 101.4, .eg = 99.6 };
    params.at(1uz) = { .mg = -2.5, .eg = 2.5 };

    const auto weights = round_params(params);

    REQUIRE(weights.values.at(0uz).mg == 101);
    REQUIRE(weights.values.at(0uz).eg == 100);
    REQUIRE(weights.values.at(1uz).mg == -3);
    REQUIRE(weights.values.at(1uz).eg == 3);
}

namespace {

// the ranges of weights that recentre() moves the averages of: the piece square tables
// (only ranks 2 to 7 for pawns) & the mobility tables
[[nodiscard]] auto recentred_tables() -> std::vector<std::pair<size_t, size_t>>
{
    using ben_bot::eval::offset_of;
    using ben_bot::eval::Term;
    using chess::board::NUM_SQUARES;

    std::vector<std::pair<size_t, size_t>> tables;

    for (auto type = 0uz; type < 6uz; ++type) {
        const auto table = offset_of(Term::Pst) + (type * NUM_SQUARES);

        if (type == static_cast<size_t>(chess::pieces::Type::Pawn))
            tables.emplace_back(table + 8uz, table + 56uz);
        else
            tables.emplace_back(table, table + NUM_SQUARES);
    }

    for (const auto term : { Term::KnightMobility, Term::BishopMobility, Term::RookMobility, Term::QueenMobility })
        tables.emplace_back(offset_of(term), offset_of(term) + count_of(term));

    return tables;
}

} // namespace

TEST_CASE("Model - re-centring the piece square & mobility tables", TAGS)
{
    using ben_bot::eval::offset_of;
    using ben_bot::eval::Term;

    const auto dataset = ben_bot::tuner::build_dataset(tuner_tests::read_positions("positions.epd"), 1uz);

    // the perturbed weights, with every table moved well off centre
    auto params = perturbed_params();

    for (const auto& [first, last] : recentred_tables()) {
        for (auto i = first; i < last; ++i) {
            params.at(i).mg += 40.;
            params.at(i).eg -= 25.;
        }
    }

    const auto recentred = ben_bot::tuner::recentre(params);

    SECTION("No eval changes")
    {
        for (const auto& entry : dataset.entries) {
            const auto before = ben_bot::tuner::model_eval(dataset, entry, params);
            const auto after  = ben_bot::tuner::model_eval(dataset, entry, recentred);

            REQUIRE(std::abs(before - after) < 1e-9);
        }
    }

    SECTION("No engine eval changes when the averages are whole numbers")
    {
        // integer weights, with each table's average a whole number, so that the
        // re-centred weights are integers too and the engine's own eval can be compared
        auto weights = DEFAULT_WEIGHTS;

        for (const auto& [first, last] : recentred_tables()) {
            // the default tables' averages aren't whole numbers, so first make them 0 exactly
            // by taking each table's sum off its first entry
            auto mg = 0;
            auto eg = 0;

            for (auto i = first; i < last; ++i) {
                mg += weights.values.at(i).mg;
                eg += weights.values.at(i).eg;
            }

            weights.values.at(first).mg -= mg;
            weights.values.at(first).eg -= eg;

            // then shift the whole table by a whole number
            for (auto i = first; i < last; ++i) {
                weights.values.at(i).mg += 30;
                weights.values.at(i).eg -= 20;
            }
        }

        const auto recentredWeights = ben_bot::tuner::round_params(ben_bot::tuner::recentre(to_params(weights)));

        REQUIRE(recentredWeights != weights);

        for (const auto& position : tuner_tests::read_positions("positions.epd")) {
            const auto trace = ben_bot::eval::evaluate_traced(position.position);

            REQUIRE(trace.score(recentredWeights) == trace.score(weights));
        }
    }

    SECTION("Each table averages 0")
    {
        for (const auto& [first, last] : recentred_tables()) {
            INFO(first);

            auto mg = 0.;
            auto eg = 0.;

            for (auto i = first; i < last; ++i) {
                mg += recentred.at(i).mg;
                eg += recentred.at(i).eg;
            }

            REQUIRE(std::abs(mg) < 1e-9);
            REQUIRE(std::abs(eg) < 1e-9);
        }
    }

    SECTION("Pawns on the first & last ranks and the other terms are left alone")
    {
        const auto tables = recentred_tables();

        for (auto i = offset_of(Term::Material) + count_of(Term::Material); i < NUM_WEIGHTS; ++i) {
            const auto inTable = std::ranges::any_of(tables, [i](const auto& table) {
                return i >= table.first and i < table.second;
            });

            if (inTable)
                continue;

            INFO(i);
            REQUIRE(std::abs(recentred.at(i).mg - params.at(i).mg) < 1e-12);
            REQUIRE(std::abs(recentred.at(i).eg - params.at(i).eg) < 1e-12);
        }
    }

    SECTION("It's idempotent")
    {
        const auto twice = ben_bot::tuner::recentre(recentred);

        for (auto i = 0uz; i < NUM_WEIGHTS; ++i) {
            REQUIRE(std::abs(twice.at(i).mg - recentred.at(i).mg) < 1e-9);
            REQUIRE(std::abs(twice.at(i).eg - recentred.at(i).eg) < 1e-9);
        }
    }
}

TEST_CASE("Model - L2 pull towards the starting weights", TAGS)
{
    const auto start  = to_params(DEFAULT_WEIGHTS);
    const auto params = perturbed_params();
    const auto mask   = ben_bot::tuner::tunable_mask();

    static constexpr auto LAMBDA = 0.25;

    Params gradient(params.size());

    const auto penalty = ben_bot::tuner::add_l2_pull(gradient, params, start, LAMBDA);

    auto expected = 0.;

    for (auto i = 0uz; i < NUM_WEIGHTS; ++i) {
        INFO(i);

        const auto mg = params.at(i).mg - start.at(i).mg;
        const auto eg = params.at(i).eg - start.at(i).eg;

        if (not mask.at(i)) {
            REQUIRE(std::abs(gradient.at(i).mg) < 1e-12);
            REQUIRE(std::abs(gradient.at(i).eg) < 1e-12);
            continue;
        }

        expected += LAMBDA * ((mg * mg) + (eg * eg));

        REQUIRE(std::abs(gradient.at(i).mg - (2. * LAMBDA * mg)) < 1e-12);
        REQUIRE(std::abs(gradient.at(i).eg - (2. * LAMBDA * eg)) < 1e-12);
    }

    REQUIRE(expected > 0.);
    REQUIRE(std::abs(penalty - expected) < 1e-9);

    // no pull at the starting weights
    Params none(start.size());

    REQUIRE(std::abs(ben_bot::tuner::add_l2_pull(none, start, start, LAMBDA)) < 1e-12);

    for (const auto& value : none) {
        REQUIRE(std::abs(value.mg) < 1e-12);
        REQUIRE(std::abs(value.eg) < 1e-12);
    }
}

TEST_CASE("Model - the written weights can replace DefaultWeights.hpp", TAGS)
{
    const auto path = std::filesystem::temp_directory_path() / "benbot_tuner_test_weights.hpp";

    // non-integer parameters that round to the defaults
    auto params = to_params(DEFAULT_WEIGHTS);

    for (auto& param : params) {
        param.mg += 0.3;
        param.eg -= 0.3;
    }

    ben_bot::tuner::write_weights(path, params);

    const auto read_file = [](const std::filesystem::path& file) {
        std::ifstream input { file };

        REQUIRE(input.is_open());

        return std::string { std::istreambuf_iterator<char> { input }, std::istreambuf_iterator<char> { } };
    };

    const auto written = read_file(path);

    std::filesystem::remove(path);

    REQUIRE(written == read_file(BENBOT_DEFAULT_WEIGHTS_HEADER));
}

TEST_CASE("Model - parse_loss", TAGS)
{
    auto loss = Loss::CrossEntropy;

    REQUIRE(ben_bot::tuner::parse_loss("mse", loss));
    REQUIRE(loss == Loss::MeanSquaredError);

    REQUIRE(ben_bot::tuner::parse_loss("ce", loss));
    REQUIRE(loss == Loss::CrossEntropy);

    REQUIRE(not ben_bot::tuner::parse_loss("l1", loss));
}
