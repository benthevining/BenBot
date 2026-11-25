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
#include <initializer_list>
#include <iterator>
#include <libbenbot/data-structures/SearchStack.hpp>
#include <libbenbot/eval/Score.hpp>
#include <libbenbot/search/Constants.hpp>
#include <optional>

inline constexpr auto TAGS { "[data-structures][SearchStack]" };

using ben_bot::eval::Score;
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

namespace {

// sets the static eval at each ply, starting from the root. nullopt means
// that node was in check
void set_evals(
    SearchStack& stack, const std::initializer_list<std::optional<int>> evals)
{
    auto ply = 0uz;

    for (const auto eval : evals) {
        stack.at(ply++)->staticEval = eval.transform([](const int value) {
            return Score { static_cast<ben_bot::eval::Value>(value) };
        });
    }
}

} // namespace

TEST_CASE("Search stack - is_improving()", TAGS)
{
    using ben_bot::is_improving;

    SearchStack stack;

    SECTION("Better than 2 plies ago")
    {
        set_evals(stack, { 10, -500, 20 });

        REQUIRE(is_improving(stack.at(2uz)));
    }

    SECTION("Worse than 2 plies ago")
    {
        set_evals(stack, { 20, -500, 10 });

        REQUIRE(not is_improving(stack.at(2uz)));
    }

    SECTION("Equal to 2 plies ago is not improving")
    {
        set_evals(stack, { 10, -500, 10 });

        REQUIRE(not is_improving(stack.at(2uz)));
    }

    SECTION("The node 1 ply ago doesn't matter")
    {
        set_evals(stack, { 10, 500, 20 });
        REQUIRE(is_improving(stack.at(2uz)));

        set_evals(stack, { 10, std::nullopt, 20 });
        REQUIRE(is_improving(stack.at(2uz)));
    }

    SECTION("In check is never improving")
    {
        set_evals(stack, { -1000, 0, std::nullopt });
        REQUIRE(not is_improving(stack.at(2uz)));

        // even with nothing to compare with
        set_evals(stack, { std::nullopt, std::nullopt, std::nullopt });
        REQUIRE(not is_improving(stack.at(2uz)));

        REQUIRE(not is_improving(stack.at(0uz)));
    }

    SECTION("In check 2 plies ago compares with 4 plies ago")
    {
        set_evals(stack, { 10, 0, std::nullopt, 0, 20 });
        REQUIRE(is_improving(stack.at(4uz)));

        set_evals(stack, { 20, 0, std::nullopt, 0, 10 });
        REQUIRE(not is_improving(stack.at(4uz)));

        set_evals(stack, { 10, 0, std::nullopt, 0, 10 });
        REQUIRE(not is_improving(stack.at(4uz)));
    }

    SECTION("6 plies ago is never used")
    {
        set_evals(stack, { 1000, 0, std::nullopt, 0, std::nullopt, 0, 10 });

        REQUIRE(is_improving(stack.at(6uz)));

        set_evals(stack, { -1000, 0, std::nullopt, 0, std::nullopt, 0, 10 });

        REQUIRE(is_improving(stack.at(6uz)));
    }

    SECTION("In check 2 & 4 plies ago is improving")
    {
        set_evals(stack, { 0, 0, std::nullopt, 0, std::nullopt, 0, -1000 });

        REQUIRE(is_improving(stack.at(6uz)));
    }

    SECTION("Near the root, the sentinels have no static eval")
    {
        // the root & ply 1 only have sentinels 2 & 4 plies back
        set_evals(stack, { -1000, -1000 });

        REQUIRE(is_improving(stack.at(0uz)));
        REQUIRE(is_improving(stack.at(1uz)));

        // ply 2 compares with the root
        set_evals(stack, { 10, 0, 0 });

        REQUIRE(not is_improving(stack.at(2uz)));

        // ply 3 compares with ply 1
        set_evals(stack, { 0, 10, 0, 20 });

        REQUIRE(is_improving(stack.at(3uz)));

        // the root was in check, & 4 plies before ply 2 is a sentinel
        set_evals(stack, { std::nullopt, 0, -1000 });

        REQUIRE(is_improving(stack.at(2uz)));

        // ply 1 was in check, & 4 plies before ply 3 is a sentinel
        set_evals(stack, { 0, std::nullopt, 0, -1000 });

        REQUIRE(is_improving(stack.at(3uz)));
    }

    SECTION("A stale eval 2 plies back is replaced when that ply is searched again")
    {
        set_evals(stack, { 10, 0, -1000 });
        REQUIRE(not is_improving(stack.at(2uz)));

        set_evals(stack, { -2000, 0, -1000 });
        REQUIRE(is_improving(stack.at(2uz)));
    }
}
