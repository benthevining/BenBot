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
}
