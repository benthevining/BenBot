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
#include <catch2/matchers/catch_matchers_all.hpp>
#include <libchess/util/inplace_any.hpp>
#include <list>
#include <utility>
#include <vector>

TEST_CASE("inplace_any", "[util]")
{
    static constexpr auto Size = 256UL;

    chess::util::inplace_any<Size> any;

    REQUIRE(any.empty());
    REQUIRE(! any.has_value());
    REQUIRE(any.try_get<float>() == nullptr);

    any.emplace<float>(1.f);

    REQUIRE(any.has_value());
    REQUIRE(! any.empty());
    REQUIRE(any.try_get<float>() != nullptr);

    REQUIRE_THAT(any.get<float>(),
        Catch::Matchers::WithinAbs(1.f, 0.000000001f));

    any.reset();

    REQUIRE(any.empty());

    any = 5;

    REQUIRE(any.get<int>() == 5);

    std::vector<double> vec;

    const auto vecType = std::type_index { typeid(vec) };

    any = std::move(vec);

    REQUIRE(any.get_stored_type() == vecType);

    chess::util::inplace_any<Size> other;

    using List = std::list<int>;

    other.emplace<List>();

    REQUIRE(other.has_value());

    auto& list = other.get<List>();

    list.emplace_back(22);

    any = std::move(other);

    REQUIRE(any.holds_type<List>());
}
