/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/util/Math.hpp>

static constexpr auto TAGS = "[util][math]";

TEST_CASE("is_even()", TAGS)
{
    using chess::math::is_even;

    STATIC_REQUIRE(is_even(0uz));
    STATIC_REQUIRE(! is_even(1uz));
    STATIC_REQUIRE(is_even(2uz));
    STATIC_REQUIRE(! is_even(3uz));
    STATIC_REQUIRE(is_even(4uz));
    STATIC_REQUIRE(! is_even(5uz));
    STATIC_REQUIRE(is_even(6uz));
    STATIC_REQUIRE(! is_even(7uz));
}
