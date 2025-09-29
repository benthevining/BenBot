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
#include <libbenbot/eval/Score.hpp>

static constexpr auto TAGS { "[eval][Score]" };

using ben_bot::eval::Score;

TEST_CASE("Score - is_mate()", TAGS)
{
    REQUIRE(not Score {}.is_mate());

    static constexpr auto mate = Score::mate(0uz);

    REQUIRE(mate.is_mate());
    REQUIRE(-mate.is_mate());
}

TEST_CASE("Score - is_winning_mate()/is_losing_mate()", TAGS)
{
    static constexpr auto mate = Score::mate(1uz);

    REQUIRE(mate.is_mate());
    STATIC_REQUIRE(mate.is_losing_mate());

    static constexpr auto flipped = -mate;

    REQUIRE(flipped.is_mate());
    STATIC_REQUIRE(flipped.is_winning_mate());
}

TEST_CASE("Score - ply_to_mate()", TAGS)
{
    for (auto ply = 0uz; ply <= 500uz; ++ply) {
        const auto mate = Score::mate(ply);

        REQUIRE(mate.is_mate());
        REQUIRE(mate.ply_to_mate() == ply);

        const auto flipped = -mate;

        REQUIRE(flipped.is_mate());
        REQUIRE(flipped.ply_to_mate() == ply);
    }
}

TEST_CASE("Score - to/from TT", TAGS)
{
    using ben_bot::eval::MATE;

    for (auto ply = 0uz; ply <= 500uz; ++ply) {
        const auto mate = Score::mate(ply);

        {
            const auto ttVal = mate.to_tt();

            REQUIRE(ttVal == -MATE);

            const auto roundTripped = Score::from_tt(ttVal, ply);

            REQUIRE(roundTripped == mate);
        }
        {
            const auto flipped = -mate;

            const auto ttVal = flipped.to_tt();

            REQUIRE(ttVal == MATE);

            const auto roundTripped = Score::from_tt(ttVal, ply);

            REQUIRE(roundTripped == flipped);
        }
    }
}
