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

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <libbenbot/search/LateMovePruning.hpp>

inline constexpr auto TAGS { "[search][LateMovePruning]" };

using ben_bot::search::LMP_MAX_DEPTH;
using ben_bot::search::lmp_move_count;

TEST_CASE("Late move pruning - move counts", TAGS)
{
    STATIC_REQUIRE(LMP_MAX_DEPTH == 7uz);

    // indexed by depthLeft
    static constexpr std::array<std::size_t, LMP_MAX_DEPTH + 1uz> improving { 4uz, 7uz, 12uz, 19uz, 28uz, 39uz, 52uz, 67uz };
    static constexpr std::array<std::size_t, LMP_MAX_DEPTH + 1uz> notImproving { 2uz, 3uz, 6uz, 9uz, 14uz, 19uz, 26uz, 33uz };

    for (auto depthLeft = 0uz; depthLeft <= LMP_MAX_DEPTH; ++depthLeft) {
        REQUIRE(lmp_move_count(depthLeft, true) == improving.at(depthLeft));
        REQUIRE(lmp_move_count(depthLeft, false) == notImproving.at(depthLeft));
    }
}

TEST_CASE("Late move pruning - the first move is never pruned", TAGS)
{
    for (auto depthLeft = 0uz; depthLeft <= LMP_MAX_DEPTH; ++depthLeft) {
        REQUIRE(lmp_move_count(depthLeft, false) >= 2uz);
        REQUIRE(lmp_move_count(depthLeft, true) >= 2uz);
    }
}

TEST_CASE("Late move pruning - nodes that aren't improving prune sooner", TAGS)
{
    for (auto depthLeft = 0uz; depthLeft <= LMP_MAX_DEPTH; ++depthLeft)
        REQUIRE(lmp_move_count(depthLeft, false) < lmp_move_count(depthLeft, true));
}

TEST_CASE("Late move pruning - more depth never prunes sooner", TAGS)
{
    for (auto depthLeft = 1uz; depthLeft <= LMP_MAX_DEPTH; ++depthLeft) {
        REQUIRE(lmp_move_count(depthLeft, false) >= lmp_move_count(depthLeft - 1uz, false));
        REQUIRE(lmp_move_count(depthLeft, true) >= lmp_move_count(depthLeft - 1uz, true));
    }
}
