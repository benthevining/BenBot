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
#include <libchess/uci/Options.hpp>
#include <string>

static constexpr auto TAGS { "[uci][options]" };

namespace uci = chess::uci;

TEST_CASE("UCI options - bool", TAGS)
{
    uci::BoolOption option { "Toggle parameter", true, "" };

    REQUIRE(option.get_declaration_string() == "option name Toggle parameter type check default true");

    REQUIRE(option.get_value());

    option.parse("name Nullmove value false\n");

    REQUIRE(option.get_value());

    option.parse("name Toggle parameter value false");

    REQUIRE(! option.get_value());
}

TEST_CASE("UCI options - int", TAGS)
{
    uci::IntOption option {
        "HashSize", 0, 100, 50, ""
    };

    REQUIRE(option.get_declaration_string() == "option name HashSize type spin default 50 min 0 max 100");

    REQUIRE(option.get_value() == 50);

    option.parse("name HashSize value 23");

    REQUIRE(option.get_value() == 23);

    option.parse("name SomeOtherParam value 42");

    REQUIRE(option.get_value() == 23);

    // test that value is clamped to legal range
    option.parse("name HashSize value 258");

    REQUIRE(option.get_value() == 100);

    option.parse("name HashSize value -4");

    REQUIRE(option.get_value() == 0);
}

TEST_CASE("UCI options - combo", TAGS)
{
    uci::ComboOption option {
        "MyEnum",
        { "One", "Two", "Three" },
        "Two", ""
    };

    REQUIRE(option.get_declaration_string() == "option name MyEnum type combo default Two var One var Two var Three");

    REQUIRE(std::string { option.get_value() } == "Two");

    option.parse("name MyEnum value One");

    REQUIRE(std::string { option.get_value() } == "One");

    option.parse("name Foo value Three");

    REQUIRE(std::string { option.get_value() } == "One");

    // should be set to default if unknown value string is received
    option.parse("name MyEnum value Four");

    REQUIRE(std::string { option.get_value() } == "Two");
}

TEST_CASE("UCI options - string", TAGS)
{
    uci::StringOption option {
        "MyString", "foo", ""
    };

    REQUIRE(std::string { option.get_value() } == "foo");

    REQUIRE(option.get_declaration_string() == "option name MyString type string default foo");

    option.parse("name MyString value bar");

    REQUIRE(std::string { option.get_value() } == "bar");

    option.parse("name OtherParam value foo");

    REQUIRE(std::string { option.get_value() } == "bar");
}

TEST_CASE("UCI options - action", TAGS)
{
    bool triggered { false };

    uci::Action action {
        "Clear Cache",
        [&triggered] { triggered = true; }, ""
    };

    REQUIRE(action.get_declaration_string() == "option name Clear Cache type button");

    action.parse("name Clear Cache");

    REQUIRE(triggered);

    triggered = false;

    action.parse("name Clear Other");

    REQUIRE(! triggered);
}
