/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Square.hpp>
#include <ranges>
#include <utility>

static constexpr auto TAGS { "[board][Bitboard]" };

using chess::board::Bitboard;

namespace bitboard_masks = chess::board::masks;

TEST_CASE("Bitboard - empty", TAGS)
{
    static constexpr Bitboard empty;

    STATIC_REQUIRE(! empty.any());
    STATIC_REQUIRE(empty.none());
    STATIC_REQUIRE(empty.count() == 0uz);
    STATIC_REQUIRE(std::cmp_equal(empty.to_int(), 0));
    STATIC_REQUIRE(std::ranges::empty(empty.squares()));
    STATIC_REQUIRE(empty == bitboard_masks::none());
}

TEST_CASE("Bitboard - all", TAGS)
{
    static constexpr auto all = bitboard_masks::all();

    STATIC_REQUIRE(all.any());
    STATIC_REQUIRE(! all.none());
    STATIC_REQUIRE(all.count() == chess::board::NUM_SQUARES);
}
