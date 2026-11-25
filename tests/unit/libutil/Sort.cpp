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
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <functional>
#include <libutil/Sort.hpp>
#include <span>
#include <utility>
#include <vector>

inline constexpr auto TAGS { "[util][sort]" };

namespace {
// a simple, fixed pseudo-random sequence, so the tests are the same everywhere
[[nodiscard]] auto make_values(const std::size_t count, const int modulo) -> std::vector<int>
{
    std::vector<int> values;

    auto state = 12345u;

    for (auto idx = 0uz; idx < count; ++idx) {
        state = (state * 1103515245u) + 12345u;
        values.push_back(static_cast<int>((state >> 16u) % static_cast<unsigned>(modulo)));
    }

    return values;
}
} // namespace

TEST_CASE("Sort - portable_sort()", TAGS)
{
    using util::sort::portable_sort;

    SECTION("Empty & single element ranges")
    {
        std::vector<int> values;

        portable_sort(std::span { values }, std::less { });

        REQUIRE(values.empty());

        values.push_back(1);

        portable_sort(std::span { values }, std::less { });

        REQUIRE(values == std::vector { 1 });
    }

    SECTION("Sorts like std::sort")
    {
        // sizes either side of the insertion sort threshold, & up to the most legal moves in a position
        for (const auto size : { 2uz, 5uz, 16uz, 17uz, 40uz, 100uz, 218uz, 1000uz }) {
            for (const auto modulo : { 2, 10, 1000000 }) {
                auto values = make_values(size, modulo);

                auto expected = values;

                std::ranges::sort(expected, std::greater { });

                portable_sort(std::span { values }, std::greater { });

                REQUIRE(values == expected);
            }
        }
    }

    SECTION("Already sorted & reversed input")
    {
        std::vector<int> values;

        for (auto idx = 0; idx < 500; ++idx)
            values.push_back(idx);

        auto reversed = values;

        std::ranges::reverse(reversed);

        portable_sort(std::span { reversed }, std::less { });

        REQUIRE(reversed == values);

        portable_sort(std::span { values }, std::less { });

        REQUIRE(std::ranges::is_sorted(values));
    }

    SECTION("Equal keys end up in the same order on every platform")
    {
        // pairs of (key, original index) sorted by key only, so the indices show where ties went
        using Pair = std::pair<int, std::size_t>;

        const auto keys = make_values(218uz, 8);

        std::vector<Pair> pairs;

        for (auto idx = 0uz; idx < keys.size(); ++idx)
            pairs.emplace_back(keys[idx], idx);

        const auto byKey = [](const Pair& first, const Pair& second) {
            return first.first > second.first;
        };

        portable_sort(std::span { pairs }, byKey);

        REQUIRE(std::ranges::is_sorted(pairs, byKey));

        // a checksum of where each original index ended up. If this changes, the order
        // of equal elements has changed, and so will bench on every platform
        auto checksum = 0uz;

        for (auto idx = 0uz; idx < pairs.size(); ++idx)
            checksum += idx * pairs[idx].second;

        REQUIRE(checksum == 2595163uz);
    }
}
