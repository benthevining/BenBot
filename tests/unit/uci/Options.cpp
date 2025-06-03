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
