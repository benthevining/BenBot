/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Masks.hpp>

static constexpr auto TAGS { "[board][Bitboard][flips]" };

TEST_CASE("Flips - vertical", TAGS)
{
    using chess::board::flips::vertical;

    SECTION("Symmetrical")
    {
        namespace ranks = chess::board::masks::ranks;

        static constexpr auto board = ranks::TWO | ranks::SEVEN;

        STATIC_REQUIRE(board == vertical(board));
    }

    SECTION("Asymmetrical")
    {
        using chess::board::Bitboard;

        static constexpr Bitboard board { 0X4100200004000 };

        STATIC_REQUIRE(vertical(board) == Bitboard { 0X40000002100400 });
    }
}
