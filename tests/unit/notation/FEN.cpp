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

using chess::notation::from_fen;
using chess::notation::to_fen;

namespace match = Catch::Matchers;

TEST_CASE("FEN - starting position", TAGS)
{
    static constexpr Position startingPos;

    static constexpr std::string_view startingFEN {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    };

    SECTION("To FEN")
    {
        const auto fen = to_fen(startingPos);

        REQUIRE_THAT(fen,
            match::Matches(startingFEN.data(), Catch::CaseSensitive::Yes));
    }

    SECTION("From FEN")
    {
        const auto pos = from_fen(startingFEN);

        REQUIRE(pos == startingPos);
    }
}

TEST_CASE("FEN - after E4")
{
    static constexpr std::string_view correctFEN {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    };

    const auto roundTripped = to_fen(from_fen(correctFEN));

    REQUIRE_THAT(roundTripped,
        match::Matches(correctFEN.data(), Catch::CaseSensitive::Yes));
}
