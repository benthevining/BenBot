/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/uci/Options.hpp>

static constexpr auto TAGS { "[uci][options]" };

namespace uci = chess::uci;

TEST_CASE("UCI options - bool", TAGS)
{
    uci::BoolOption option { "Toggle parameter", true };

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
        "HashSize", 0, 100, 50
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
