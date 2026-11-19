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

inline constexpr auto TAGS { "[eval][Score]" };

using ben_bot::eval::Score;

TEST_CASE("Score - is_mate()", TAGS)
{
    REQUIRE_FALSE(Score { }.is_mate());

    static constexpr auto mate = Score::mate(0uz);

    REQUIRE(mate.is_mate());
    REQUIRE((-mate).is_mate());
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
    for (auto matePly = 0uz; matePly <= 500uz; ++matePly) {
        const auto mate = Score::mate(matePly);

        for (auto storePly = 0uz; storePly <= matePly; ++storePly) {
            const auto distance = matePly - storePly;

            {
                const auto ttVal = mate.to_tt(storePly);

                // stored value is relative to the stored node
                REQUIRE(ttVal == Score::mate(distance));

                REQUIRE(Score::from_tt(ttVal, storePly) == mate);

                // probing at a different ply preserves the distance to mate
                for (auto probePly = 0uz; probePly <= 10uz; ++probePly) {
                    const auto probed = Score::from_tt(ttVal, probePly);

                    REQUIRE(probed.is_losing_mate());
                    REQUIRE(probed.ply_to_mate() == probePly + distance);
                }
            }
            {
                const auto flipped = -mate;

                const auto ttVal = flipped.to_tt(storePly);

                REQUIRE(ttVal == -Score::mate(distance));

                REQUIRE(Score::from_tt(ttVal, storePly) == flipped);

                for (auto probePly = 0uz; probePly <= 10uz; ++probePly) {
                    const auto probed = Score::from_tt(ttVal, probePly);

                    REQUIRE(probed.is_winning_mate());
                    REQUIRE(probed.ply_to_mate() == probePly + distance);
                }
            }
        }
    }
}

TEST_CASE("Score - to/from TT (non-mate)", TAGS)
{
    for (const auto value : { -500, -1, 0, 1, 500 }) {
        const Score score { static_cast<ben_bot::eval::Value>(value) };

        for (auto ply = 0uz; ply <= 10uz; ++ply) {
            REQUIRE(score.to_tt(ply) == score.value);
            REQUIRE(Score::from_tt(score.to_tt(ply), ply) == score);
        }
    }
}
