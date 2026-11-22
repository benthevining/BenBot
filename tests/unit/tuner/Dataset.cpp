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
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <span>
#include <sstream>
#include <string>
#include <tuner/Dataset.hpp>
#include <tuner/Model.hpp>
#include <vector>

namespace {

using ben_bot::tuner::build_dataset;
using ben_bot::tuner::load_dataset;
using ben_bot::tuner::LoadOptions;
using ben_bot::tuner::LoadStats;
using ben_bot::tuner::parse_labelled_line;

constexpr auto TAGS = "[tuner][dataset]";

[[nodiscard]] auto load_file(const std::string& name, const LoadOptions& options, LoadStats& stats)
{
    const std::array files { tuner_tests::data_file(name) };

    return load_dataset(files, options, stats);
}

} // namespace

TEST_CASE("Dataset - parse_labelled_line", TAGS)
{
    const auto result_of = [](const std::string& line) {
        const auto parsed = parse_labelled_line(line);

        INFO(line);
        REQUIRE(parsed.has_value());

        return parsed->result;
    };

    static constexpr auto START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";

    SECTION("Bracketed numbers")
    {
        REQUIRE(result_of(std::string { START } + " 0 1 [1.0]") == 1.);
        REQUIRE(result_of(std::string { START } + " 0 1 [0.5]") == 0.5);
        REQUIRE(result_of(std::string { START } + " 0 1 [0.0]") == 0.);
        REQUIRE(result_of(std::string { START } + " [1]") == 1.);
        REQUIRE(result_of(std::string { START } + " [0]") == 0.);
        REQUIRE(result_of(std::string { START } + " [ 0.5 ]") == 0.5);
    }

    SECTION("Quoted results")
    {
        REQUIRE(result_of(std::string { START } + R"( c9 "1-0";)") == 1.);
        REQUIRE(result_of(std::string { START } + R"( c9 "1/2-1/2";)") == 0.5);
        REQUIRE(result_of(std::string { START } + R"( c9 "0-1";)") == 0.);
        REQUIRE(result_of(std::string { START } + R"( 0 1 "1-0")") == 1.);
    }

    SECTION("Bare results")
    {
        REQUIRE(result_of(std::string { START } + " 1-0") == 1.);
        REQUIRE(result_of(std::string { START } + " 0 1 1/2-1/2") == 0.5);
        REQUIRE(result_of(std::string { START } + " 0-1;") == 0.);
    }

    SECTION("The FEN is parsed, with or without move counters")
    {
        const auto parsed = parse_labelled_line("8/5k2/8/8/8/8/5K2/4R3 b - - 12 40 [0.0]");

        REQUIRE(parsed.has_value());
        REQUIRE(chess::notation::to_fen(parsed->position) == "8/5k2/8/8/8/8/5K2/4R3 b - - 12 40");

        const auto withoutCounters = parse_labelled_line(R"(8/5k2/8/8/8/8/5K2/4R3 b - - c9 "1-0";)");

        REQUIRE(withoutCounters.has_value());
        REQUIRE(withoutCounters->position.sideToMove == chess::pieces::Color::Black);
        REQUIRE(withoutCounters->position.hash == parsed->position.hash);
    }

    SECTION("Errors")
    {
        REQUIRE(not parse_labelled_line(std::string { START }).has_value());            // no result
        REQUIRE(not parse_labelled_line(std::string { START } + " [2.0]").has_value()); // out of range
        REQUIRE(not parse_labelled_line(std::string { START } + " [-1]").has_value());
        REQUIRE(not parse_labelled_line(std::string { START } + " [win]").has_value());
        REQUIRE(not parse_labelled_line(std::string { START } + R"( c9 "2-0";)").has_value());
        REQUIRE(not parse_labelled_line(std::string { START } + " [1.0").has_value());
        REQUIRE(not parse_labelled_line("8/8/8 w [1.0]").has_value());
        REQUIRE(not parse_labelled_line("").has_value());
    }
}

TEST_CASE("Dataset - loading skips in-check, duplicate & bad lines", TAGS)
{
    LoadStats stats;

    const auto [training, validation] = load_file("formats.epd", LoadOptions { .validationFraction = 0. }, stats);

    REQUIRE(stats.lines == 13uz);
    REQUIRE(stats.skipped == 2uz);
    REQUIRE(stats.parseErrors == 2uz);
    REQUIRE(stats.firstErrors.size() == 2uz);
    REQUIRE(stats.inCheck == 2uz);
    REQUIRE(stats.duplicates == 2uz);
    REQUIRE(stats.accepted == 5uz);

    REQUIRE(validation.empty());
    REQUIRE(training.size() == 5uz);

    // the first occurrence of each duplicate is kept, in file order
    std::vector<float> results;

    std::ranges::transform(training.entries, std::back_inserter(results), &ben_bot::tuner::Entry::result);

    REQUIRE(results == std::vector { 1.f, 0.5f, 0.f, 1.f, 1.f });
}

TEST_CASE("Dataset - the limit stops loading early", TAGS)
{
    LoadStats stats;

    const auto [training, validation] = load_file(
        "formats.epd", LoadOptions { .validationFraction = 0., .limit = 3uz }, stats);

    REQUIRE(stats.accepted == 3uz);
    REQUIRE(training.size() == 3uz);
}

TEST_CASE("Dataset - validation split", TAGS)
{
    const auto split_with = [](const std::uint64_t seed, const std::size_t numThreads) {
        LoadStats stats;

        return load_file(
            "positions.epd",
            LoadOptions { .validationFraction = 0.3, .seed = seed, .numThreads = numThreads },
            stats);
    };

    const auto first = split_with(1, 1uz);

    REQUIRE(first.training.size() + first.validation.size() == 100uz);
    REQUIRE(first.training.size() > 50uz);
    REQUIRE(first.validation.size() > 10uz);

    // the same seed gives the same split, whatever the number of threads
    const auto again = split_with(1, 4uz);

    REQUIRE(again.training.size() == first.training.size());
    REQUIRE(again.training.tuples.size() == first.training.tuples.size());

    for (auto i = 0uz; i < first.training.size(); ++i) {
        REQUIRE(again.training.entries.at(i).firstTuple == first.training.entries.at(i).firstTuple);
        REQUIRE(again.training.entries.at(i).numTuples == first.training.entries.at(i).numTuples);
    }

    // a different seed gives a different split
    const auto other = split_with(2, 1uz);

    REQUIRE(other.training.size() + other.validation.size() == 100uz);
    REQUIRE(other.training.tuples.size() != first.training.tuples.size());
    REQUIRE(other.validation.tuples.size() != first.validation.tuples.size());
}

TEST_CASE("Dataset - a missing file is an error", TAGS)
{
    LoadStats stats;

    REQUIRE_THROWS(load_file("does-not-exist.epd", LoadOptions { }, stats));
}

TEST_CASE("Dataset - the traced dataset rebuilds the eval", TAGS)
{
    using ben_bot::tuner::model_eval;

    const auto positions = tuner_tests::read_positions("positions.epd");

    const auto dataset = build_dataset(positions, 3uz);

    REQUIRE(dataset.size() == positions.size());

    const auto params = ben_bot::tuner::to_params(ben_bot::eval::DEFAULT_WEIGHTS);

    const auto mask = ben_bot::tuner::tunable_mask();

    for (auto i = 0uz; i < positions.size(); ++i) {
        const auto& position = positions.at(i).position;
        const auto& entry    = dataset.entries.at(i);

        INFO(chess::notation::to_fen(position));

        REQUIRE(std::abs(static_cast<double>(entry.result) - positions.at(i).result) < 1e-6);

        // the tuples are sparse, sorted, and only for tunable weights
        const auto tuples = dataset.tuples_of(entry);

        REQUIRE(not tuples.empty());

        for (const auto& tuple : tuples) {
            REQUIRE(tuple.coeff != 0);
            REQUIRE(mask.at(tuple.index));
        }

        REQUIRE(std::ranges::is_sorted(tuples, { }, &ben_bot::tuner::Tuple::index));

        // the model is the eval, before it's truncated to an integer
        auto eval = model_eval(dataset, entry, params);

        if (position.sideToMove == chess::pieces::Color::Black)
            eval = -eval;

        REQUIRE(std::abs(eval - static_cast<double>(ben_bot::eval::evaluate(position))) < 1.);
    }
}

TEST_CASE("Dataset - append offsets the tuples", TAGS)
{
    const auto positions = tuner_tests::read_positions("positions.epd");

    const auto whole = build_dataset(positions, 1uz);

    auto joined = build_dataset(std::span { positions }.first(40uz), 1uz);

    joined.append(build_dataset(std::span { positions }.subspan(40uz), 1uz));

    REQUIRE(joined.size() == whole.size());
    REQUIRE(joined.tuples.size() == whole.tuples.size());

    for (auto i = 0uz; i < whole.size(); ++i) {
        REQUIRE(joined.entries.at(i).firstTuple == whole.entries.at(i).firstTuple);
        REQUIRE(joined.entries.at(i).numTuples == whole.entries.at(i).numTuples);
    }
}

TEST_CASE("Dataset - loading from a stream", TAGS)
{
    std::istringstream input { "8/5k2/8/8/8/8/5K2/4R3 b - - [1]\n\n4k3/8/8/8/8/8/3P4/4K3 w - - 0 1 1-0\n" };

    LoadStats stats;

    const auto [training, validation] = load_dataset(input, LoadOptions { .validationFraction = 0. }, stats);

    REQUIRE(stats.lines == 3uz);
    REQUIRE(stats.accepted == 2uz);
    REQUIRE(training.size() == 2uz);
}
