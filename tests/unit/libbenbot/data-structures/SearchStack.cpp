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

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <iterator>
#include <libbenbot/data-structures/SearchStack.hpp>
#include <libbenbot/search/Constants.hpp>

inline constexpr auto TAGS { "[data-structures][SearchStack]" };

using ben_bot::SearchStack;

TEST_CASE("Search stack - entries are contiguous by ply", TAGS)
{
    SearchStack stack;

    for (auto ply = 0uz; ply < ben_bot::search::MAX_PLY; ++ply)
        REQUIRE(std::next(stack.at(ply)) == stack.at(ply + 1uz));

    REQUIRE(std::distance(stack.at(0uz), stack.at(ben_bot::search::MAX_PLY))
            == static_cast<std::ptrdiff_t>(ben_bot::search::MAX_PLY));
}

TEST_CASE("Search stack - the root & the sentinels before it start out empty", TAGS)
{
    const SearchStack stack;

    for (auto pliesBack = 0uz; pliesBack <= SearchStack::SENTINELS; ++pliesBack) {
        const auto* const entry = std::prev(stack.at(0uz), static_cast<std::ptrdiff_t>(pliesBack));

        REQUIRE(entry->move.is_null());
        REQUIRE(not entry->staticEval.has_value());
    }
}
