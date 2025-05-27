/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>

static constexpr auto TAGS { "[board][Bitboard][fills]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

namespace fills = chess::board::fills;

/*
white pawns         black pawns
. . . . . . . .     . . . . . . . .
. . . . . . . .     . 1 . . . 1 1 .
. . . . . . . .     1 . 1 . . . . 1
. . . . . . . .     . . . 1 . . . .
1 . . . . 1 . .     . . . . . . . .
. . 1 . . . . .     . . . . . . . .
. 1 1 . . . 1 1     . . . . . . . .
. . . . . . . .     . . . . . . . .

North fill
white frontfill     black rearfill
1 1 1 . . 1 1 1     1 1 1 1 . 1 1 1
1 1 1 . . 1 1 1     1 1 1 1 . 1 1 1
1 1 1 . . 1 1 1     1 . 1 1 . . . 1
1 1 1 . . 1 1 1     . . . 1 . . . .
1 1 1 . . 1 1 1     . . . . . . . .
. 1 1 . . . 1 1      . . . . . . . .
. 1 1 . . . 1 1      . . . . . . . .
. . . . . . . .      . . . . . . . .

South fill
white rearfill      black frontfill
. . . . . . . .     . . . . . . . .
. . . . . . . .     . 1 . . . 1 1 .
. . . . . . . .     1 1 1 . . 1 1 1
. . . . . . . .     1 1 1 1 . 1 1 1
1 . . . . 1 . .     1 1 1 1 . 1 1 1
1 . 1 . . 1 . .     1 1 1 1 . 1 1 1
1 1 1 . . 1 1 1     1 1 1 1 . 1 1 1
1 1 1 . . 1 1 1     1 1 1 1 . 1 1 1
 */

[[nodiscard, gnu::const]] constexpr Bitboard white_pawns_start() noexcept
{
    Bitboard board;

    board.set(Square { File::A, Rank::Four });
    board.set(Square { File::B, Rank::Two });
    board.set(Square { File::C, Rank::Two });
    board.set(Square { File::C, Rank::Three });
    board.set(Square { File::F, Rank::Four });
    board.set(Square { File::G, Rank::Two });
    board.set(Square { File::H, Rank::Two });

    return board;
}

[[nodiscard, gnu::const]] constexpr Bitboard black_pawns_start() noexcept
{
    Bitboard board;

    board.set(Square { File::A, Rank::Six });
    board.set(Square { File::B, Rank::Seven });
    board.set(Square { File::C, Rank::Six });
    board.set(Square { File::D, Rank::Five });
    board.set(Square { File::F, Rank::Seven });
    board.set(Square { File::G, Rank::Seven });
    board.set(Square { File::H, Rank::Six });

    return board;
}

TEST_CASE("Fills - north", TAGS)
{
    SECTION("White frontfill")
    {
        static constexpr auto start = white_pawns_start();

        static constexpr auto filled = fills::north(start);

        STATIC_REQUIRE(filled.count() == 38uz);

        STATIC_REQUIRE(filled == Bitboard { 0Xe7e7e7e7e7c6c600 });
    }

    SECTION("Black rearfill")
    {
        static constexpr auto start = black_pawns_start();

        static constexpr auto filled = fills::north(start);

        STATIC_REQUIRE(filled.count() == 19uz);

        STATIC_REQUIRE(filled == Bitboard { 0Xefef8d0800000000 });
    }
}

TEST_CASE("Fills - south", TAGS)
{
    SECTION("White rearfill")
    {
        static constexpr auto start = white_pawns_start();

        static constexpr auto filled = fills::south(start);

        STATIC_REQUIRE(filled.count() == 17uz);

        STATIC_REQUIRE(filled == Bitboard { 0X2125e7e7 });
    }

    SECTION("Black frontfill")
    {
        static constexpr auto start = black_pawns_start();

        static constexpr auto filled = fills::south(start);

        STATIC_REQUIRE(filled == Bitboard { 0X62e7efefefefef });
    }
}

TEST_CASE("Fills - file", TAGS)
{
    SECTION("White file fill")
    {
        static constexpr auto start = white_pawns_start();

        static constexpr auto filled = fills::file(start);

        STATIC_REQUIRE(filled == Bitboard { 0Xe7e7e7e7e7e7e7e7 });
    }

    SECTION("Black file fill")
    {
        static constexpr auto start = black_pawns_start();

        static constexpr auto filled = fills::file(start);

        STATIC_REQUIRE(filled == Bitboard { 0Xefefefefefefefef });
    }
}
