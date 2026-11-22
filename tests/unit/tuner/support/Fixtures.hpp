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

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <tuner/Dataset.hpp>
#include <utility>
#include <vector>

namespace tuner_tests {

/** Returns the path of a file in the test data directory. */
[[nodiscard]] inline auto data_file(const std::string& name) -> std::filesystem::path
{
    return std::filesystem::path { BENBOT_TUNER_TEST_DATA_DIR } / name;
}

/** Reads every labelled position in a test data file, failing the test if any line doesn't parse. */
[[nodiscard]] inline auto read_positions(const std::string& name) -> std::vector<ben_bot::tuner::LabelledPosition>
{
    std::ifstream file { data_file(name) };

    REQUIRE(file.is_open());

    std::vector<ben_bot::tuner::LabelledPosition> positions;

    std::string line;

    while (std::getline(file, line)) {
        auto parsed = ben_bot::tuner::parse_labelled_line(line);

        INFO(line);
        REQUIRE(parsed.has_value());

        positions.push_back(std::move(*parsed));
    }

    return positions;
}

} // namespace tuner_tests
