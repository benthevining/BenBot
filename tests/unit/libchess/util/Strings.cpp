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
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <string_view>
#include <vector>

static constexpr auto TAGS { "[util][strings]" };

TEST_CASE("Strings - trim()", TAGS)
{
    using chess::util::trim;

    REQUIRE(trim({}).empty());
    REQUIRE(trim(" ").empty());
    REQUIRE(trim("\n").empty());

    REQUIRE(trim(" 123456") == "123456");
    REQUIRE(trim("\n123456") == "123456");

    REQUIRE(trim("123456 ") == "123456");
    REQUIRE(trim("123456\n") == "123456");

    REQUIRE(trim(" 123456 ") == "123456");
    REQUIRE(trim("\n123456\n") == "123456");
}

TEST_CASE("Strings - split_at_first_space()", TAGS)
{
    using chess::util::split_at_first_space;

    SECTION("Empty string")
    {
        const auto [before, after] = split_at_first_space({});

        REQUIRE(before.empty());
        REQUIRE(after.empty());
    }

    SECTION("No whitespace")
    {
        const auto [before, after] = split_at_first_space("abcdef");

        REQUIRE(before == "abcdef");
        REQUIRE(after.empty());
    }

    SECTION("Starts with whitespace")
    {
        const auto [before, after] = split_at_first_space(" abcdef");

        REQUIRE(before.empty());
        REQUIRE(after == "abcdef");
    }

    SECTION("Two words")
    {
        const auto [before, after] = split_at_first_space("123 456");

        REQUIRE(before == "123");
        REQUIRE(after == "456");
    }

    SECTION("Second word contains spaces")
    {
        const auto [before, after] = split_at_first_space("123 456 789");

        REQUIRE(before == "123");
        REQUIRE(after == "456 789");
    }
}

TEST_CASE("Strings - split_at_first_space_or_newline()", TAGS)
{
    using chess::util::split_at_first_space_or_newline;

    SECTION("Empty string")
    {
        const auto [before, after] = split_at_first_space_or_newline({});

        REQUIRE(before.empty());
        REQUIRE(after.empty());
    }

    SECTION("No whitespace")
    {
        const auto [before, after] = split_at_first_space_or_newline("abcdef");

        REQUIRE(before == "abcdef");
        REQUIRE(after.empty());
    }

    SECTION("Starts with whitespace")
    {
        {
            const auto [before, after] = split_at_first_space_or_newline(" abcdef");

            REQUIRE(before.empty());
            REQUIRE(after == "abcdef");
        }
        {
            const auto [before, after] = split_at_first_space_or_newline("\nabcdef");

            REQUIRE(before.empty());
            REQUIRE(after == "abcdef");
        }
    }

    SECTION("Two words")
    {
        {
            const auto [before, after] = split_at_first_space_or_newline("123 456");

            REQUIRE(before == "123");
            REQUIRE(after == "456");
        }
        {
            const auto [before, after] = split_at_first_space_or_newline("123\n456");

            REQUIRE(before == "123");
            REQUIRE(after == "456");
        }
    }

    SECTION("Second word contains spaces")
    {
        {
            const auto [before, after] = split_at_first_space_or_newline("123 456 789");

            REQUIRE(before == "123");
            REQUIRE(after == "456 789");
        }
        {
            const auto [before, after] = split_at_first_space_or_newline("123\n456\n789");

            REQUIRE(before == "123");
            REQUIRE(after == "456\n789");
        }
    }
}

TEST_CASE("Strings - find_matching_close_paren()", TAGS)
{
    using chess::util::find_matching_close_paren;

    REQUIRE_FALSE(find_matching_close_paren("("));
    REQUIRE_FALSE(find_matching_close_paren("(()"));

    REQUIRE(find_matching_close_paren("()").value() == 1uz);
    REQUIRE(find_matching_close_paren("(())").value() == 3uz);

    REQUIRE(find_matching_close_paren("(123)34f3g3g").value() == 4uz);
    REQUIRE(find_matching_close_paren("(3(ervev)1424)wcevev").value() == 13uz);
}

TEST_CASE("Strings - lines_view()", TAGS)
{
    auto lines_vector = [](const std::string_view input) {
        return chess::util::lines_view(input)
             | std::ranges::to<std::vector>();
    };

    REQUIRE(lines_vector({}).empty());

    SECTION("Single line")
    {
        const auto lines = lines_vector("123456");

        REQUIRE(lines.size() == 1uz);
        REQUIRE(lines.front() == "123456");
    }

    SECTION("2 lines")
    {
        const auto lines = lines_vector("123\n456");

        REQUIRE(lines.size() == 2uz);

        REQUIRE(lines.front() == "123");
        REQUIRE(lines.back() == "456");
    }
}

TEST_CASE("Strings - words_view()", TAGS)
{
    auto words_vector = [](const std::string_view input) {
        return chess::util::words_view(input)
             | std::ranges::to<std::vector>();
    };

    REQUIRE(words_vector({}).empty());

    SECTION("Single word")
    {
        const auto words = words_vector("123456");

        REQUIRE(words.size() == 1uz);
        REQUIRE(words.front() == "123456");
    }

    SECTION("2 words")
    {
        const auto words = words_vector("123 456");

        REQUIRE(words.size() == 2uz);

        REQUIRE(words.front() == "123");
        REQUIRE(words.back() == "456");
    }
}
