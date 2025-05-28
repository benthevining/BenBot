/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <string_view>

static constexpr auto TAGS { "[notation][FEN]" };

using chess::game::Position;

namespace match = Catch::Matchers;

static constexpr std::string_view startingFEN {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
};

TEST_CASE("Position to FEN", TAGS)
{
    using chess::notation::to_fen;

    SECTION("Starting position")
    {
        static constexpr Position startingPos;

        static const auto fen = to_fen(startingPos);

        REQUIRE_THAT(fen,
            match::Matches(startingFEN.data(), Catch::CaseSensitive::Yes));
    }
}

TEST_CASE("Position from FEN", TAGS)
{
    using chess::notation::from_fen;

    SECTION("Starting position")
    {
        static constexpr Position startingPos;

        const auto position = from_fen(startingFEN);

        REQUIRE(position == startingPos);
    }
}
