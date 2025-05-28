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
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <ranges>
#include <utility>

static constexpr auto TAGS { "[board][Bitboard]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

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

    STATIC_REQUIRE(empty.first() == chess::board::NUM_SQUARES);
    STATIC_REQUIRE(empty.last() == chess::board::NUM_SQUARES);
}

TEST_CASE("Bitboard - all", TAGS)
{
    static constexpr auto all = bitboard_masks::all();

    STATIC_REQUIRE(all.any());
    STATIC_REQUIRE(! all.none());
    STATIC_REQUIRE(all.count() == chess::board::NUM_SQUARES);

    STATIC_REQUIRE(all.first() == 0uz);
    STATIC_REQUIRE(all.last() == 63uz);
}

TEST_CASE("Bitboard - first()", TAGS)
{
    SECTION("A1")
    {
        static constexpr Bitboard board { Square { File::A, Rank::One } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.first() == 0uz);
    }

    SECTION("B1")
    {
        static constexpr Bitboard board { Square { File::B, Rank::One } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.first() == 1uz);
    }

    SECTION("A2")
    {
        static constexpr Bitboard board { Square { File::A, Rank::Two } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.first() == 8uz);
    }

    SECTION("H8")
    {
        static constexpr Bitboard board { Square { File::H, Rank::Eight } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.first() == 63uz);
    }
}

TEST_CASE("Bitboard - last()", TAGS)
{
    SECTION("H8")
    {
        static constexpr Bitboard board { Square { File::H, Rank::Eight } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.last() == 63uz);
    }

    SECTION("G8")
    {
        static constexpr Bitboard board { Square { File::G, Rank::Eight } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.last() == 62uz);
    }

    SECTION("H7")
    {
        static constexpr Bitboard board { Square { File::H, Rank::Seven } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.last() == 55uz);
    }

    SECTION("A1")
    {
        static constexpr Bitboard board { Square { File::A, Rank::One } };

        STATIC_REQUIRE(board.count() == 1uz);
        STATIC_REQUIRE(board.last() == 0uz);
    }
}
