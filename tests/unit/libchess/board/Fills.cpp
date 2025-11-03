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
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Fills.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>

inline constexpr auto TAGS { "[board][Bitboard][fills]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::pieces::Color;

namespace fills = chess::board::fills;

// for _bb bitboard literal
using namespace chess::board::literals; // NOLINT

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

[[nodiscard, gnu::const]] consteval Bitboard white_pawns_start() noexcept
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

[[nodiscard, gnu::const]] consteval Bitboard black_pawns_start() noexcept
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

        STATIC_REQUIRE(filled == 0Xe7e7e7e7e7c6c600_bb);

        STATIC_REQUIRE(filled == fills::pawn_front<Color::White>(start));
    }

    SECTION("Black rearfill")
    {
        static constexpr auto start = black_pawns_start();

        static constexpr auto filled = fills::north(start);

        STATIC_REQUIRE(filled.count() == 19uz);

        STATIC_REQUIRE(filled == 0Xefef8d0800000000_bb);

        STATIC_REQUIRE(filled == fills::pawn_rear<Color::Black>(start));
    }
}

TEST_CASE("Fills - south", TAGS)
{
    SECTION("White rearfill")
    {
        static constexpr auto start = white_pawns_start();

        static constexpr auto filled = fills::south(start);

        STATIC_REQUIRE(filled.count() == 17uz);

        STATIC_REQUIRE(filled == 0X2125e7e7_bb);

        STATIC_REQUIRE(filled == fills::pawn_rear<Color::White>(start));
    }

    SECTION("Black frontfill")
    {
        static constexpr auto start = black_pawns_start();

        static constexpr auto filled = fills::south(start);

        STATIC_REQUIRE(filled == 0X62e7efefefefef_bb);

        STATIC_REQUIRE(filled == fills::pawn_front<Color::Black>(start));
    }
}

TEST_CASE("Fills - file", TAGS)
{
    SECTION("White file fill")
    {
        static constexpr auto filled = fills::file(white_pawns_start());

        STATIC_REQUIRE(filled == 0Xe7e7e7e7e7e7e7e7_bb);
    }

    SECTION("Black file fill")
    {
        static constexpr auto filled = fills::file(black_pawns_start());

        STATIC_REQUIRE(filled == 0Xefefefefefefefef_bb);
    }
}

TEST_CASE("Fills - east", TAGS)
{
    static constexpr auto filled = fills::east(0X8004001000000240_bb);

    STATIC_REQUIRE(filled == 0X80fc00f00000fec0_bb);
}

TEST_CASE("Fills - west", TAGS)
{
    static constexpr auto filled = fills::west(0X8000080004400201_bb);

    STATIC_REQUIRE(filled == 0Xff000f00077f0301_bb);
}

TEST_CASE("Fills - rank", TAGS)
{
    static constexpr auto filled = fills::rank(0Xc000181000200002_bb);

    STATIC_REQUIRE(filled == 0Xff00ffff00ff00ff_bb);
}

TEST_CASE("Fills - northeast", TAGS)
{
    static constexpr auto filled = fills::northeast(0X10000000044000_bb);

    STATIC_REQUIRE(filled == 0Xa050201008844000_bb);
}

TEST_CASE("Fills - southeast", TAGS)
{
    static constexpr auto filled = fills::southeast(0X41000800000020_bb);

    STATIC_REQUIRE(filled == 0X41820c183060e0_bb);
}

TEST_CASE("Fills - northwest", TAGS)
{
    static constexpr auto filled = fills::northwest(0X80080040000_bb);

    STATIC_REQUIRE(filled == 0Xa14284182040000_bb);
}

TEST_CASE("Fills - southwest", TAGS)
{
    static constexpr auto filled = fills::southwest(0X8000002100100_bb);

    STATIC_REQUIRE(filled == 0X8040203110904_bb);
}
