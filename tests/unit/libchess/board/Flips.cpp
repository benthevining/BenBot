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
#include <libchess/board/Flips.hpp>
#include <libchess/board/Masks.hpp>

static constexpr auto TAGS { "[board][Bitboard][flips]" };

using chess::board::Bitboard;

// for _bb bitboard literal
using namespace chess::board::literals; // NOLINT

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
        static constexpr auto board = 0X4100200004000_bb;

        STATIC_REQUIRE(vertical(board) == 0X40000002100400_bb);
    }
}

TEST_CASE("Flips - horizontal", TAGS)
{
    using chess::board::flips::horizontal;

    SECTION("Symmetrical")
    {
        namespace files = chess::board::masks::files;

        static constexpr auto board = files::B | files::G;

        STATIC_REQUIRE(board == horizontal(board));
    }

    SECTION("Asymmetrical")
    {
        static constexpr auto board = 0X1e2222120e0a1222_bb;

        STATIC_REQUIRE(horizontal(board) == 0X7844444870504844_bb);
    }
}
