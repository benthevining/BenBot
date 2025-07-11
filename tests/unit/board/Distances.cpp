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

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <magic_enum/magic_enum.hpp>

static constexpr auto TAGS { "[board][Square][distances]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

using magic_enum::enum_values;

TEST_CASE("File distance", TAGS)
{
    SECTION("A/B files")
    {
        for (const auto rank : enum_values<Rank>()) {
            const Square aFile { File::A, rank };
            const Square bFile { File::B, rank };

            REQUIRE(file_distance(aFile, aFile) == 0uz);
            REQUIRE(file_distance(bFile, bFile) == 0uz);

            REQUIRE(file_distance(aFile, bFile) == 1uz);
            REQUIRE(file_distance(bFile, aFile) == 1uz);
        }
    }

    SECTION("G/H files")
    {
        for (const auto rank : enum_values<Rank>()) {
            const Square gFile { File::G, rank };
            const Square HFile { File::H, rank };

            REQUIRE(file_distance(gFile, gFile) == 0uz);
            REQUIRE(file_distance(HFile, HFile) == 0uz);

            REQUIRE(file_distance(gFile, HFile) == 1uz);
            REQUIRE(file_distance(HFile, gFile) == 1uz);
        }
    }

    SECTION("A/H files")
    {
        for (const auto rank : enum_values<Rank>()) {
            const Square aFile { File::A, rank };
            const Square hFile { File::H, rank };

            REQUIRE(file_distance(aFile, hFile) == 7uz);
            REQUIRE(file_distance(hFile, aFile) == 7uz);
        }
    }
}

TEST_CASE("Rank distance", TAGS)
{
    SECTION("1/2")
    {
        for (const auto file : enum_values<File>()) {
            const Square rank1 { file, Rank::One };
            const Square rank2 { file, Rank::Two };

            REQUIRE(rank_distance(rank1, rank1) == 0uz);
            REQUIRE(rank_distance(rank2, rank2) == 0uz);

            REQUIRE(rank_distance(rank1, rank2) == 1uz);
            REQUIRE(rank_distance(rank2, rank1) == 1uz);
        }
    }

    SECTION("7/8")
    {
        for (const auto file : enum_values<File>()) {
            const Square rank7 { file, Rank::Seven };
            const Square rank8 { file, Rank::Eight };

            REQUIRE(rank_distance(rank7, rank7) == 0uz);
            REQUIRE(rank_distance(rank8, rank8) == 0uz);

            REQUIRE(rank_distance(rank7, rank8) == 1uz);
            REQUIRE(rank_distance(rank8, rank7) == 1uz);
        }
    }

    SECTION("1/8")
    {
        for (const auto file : enum_values<File>()) {
            const Square rank1 { file, Rank::One };
            const Square rank8 { file, Rank::Eight };

            REQUIRE(rank_distance(rank1, rank8) == 7uz);
            REQUIRE(rank_distance(rank8, rank1) == 7uz);
        }
    }
}

TEST_CASE("Manhattan distance", TAGS)
{
    SECTION("A1/H8")
    {
        static constexpr Square a1 { File::A, Rank::One };
        static constexpr Square h8 { File::H, Rank::Eight };

        STATIC_REQUIRE(manhattan_distance(a1, a1) == 0uz);
        STATIC_REQUIRE(manhattan_distance(h8, h8) == 0uz);

        STATIC_REQUIRE(manhattan_distance(a1, h8) == 14uz);
        STATIC_REQUIRE(manhattan_distance(h8, a1) == 14uz);
    }

    SECTION("A8/H1")
    {
        static constexpr Square a8 { File::A, Rank::Eight };
        static constexpr Square h1 { File::H, Rank::One };

        STATIC_REQUIRE(manhattan_distance(a8, a8) == 0uz);
        STATIC_REQUIRE(manhattan_distance(h1, h1) == 0uz);

        STATIC_REQUIRE(manhattan_distance(a8, h1) == 14uz);
        STATIC_REQUIRE(manhattan_distance(h1, a8) == 14uz);
    }
}

TEST_CASE("Center Manhattan distance", TAGS)
{
    STATIC_REQUIRE(center_manhattan_distance(Square { File::D, Rank::Four }) == 0uz);
    STATIC_REQUIRE(center_manhattan_distance(Square { File::D, Rank::Five }) == 0uz);
    STATIC_REQUIRE(center_manhattan_distance(Square { File::E, Rank::Four }) == 0uz);
    STATIC_REQUIRE(center_manhattan_distance(Square { File::E, Rank::Five }) == 0uz);

    STATIC_REQUIRE(center_manhattan_distance(Square { File::C, Rank::Two }) == 3uz);

    STATIC_REQUIRE(center_manhattan_distance(Square { File::A, Rank::Eight }) == 6uz);
}

TEST_CASE("Chebyshev distance", TAGS)
{
    static constexpr Square a1 { File::A, Rank::One };
    static constexpr Square a8 { File::A, Rank::Eight };
    static constexpr Square h1 { File::H, Rank::One };
    static constexpr Square h8 { File::H, Rank::Eight };

    STATIC_REQUIRE(chebyshev_distance(a1, a1) == 0uz);
    STATIC_REQUIRE(chebyshev_distance(a8, a8) == 0uz);
    STATIC_REQUIRE(chebyshev_distance(h1, h1) == 0uz);
    STATIC_REQUIRE(chebyshev_distance(h8, h8) == 0uz);

    STATIC_REQUIRE(chebyshev_distance(a1, a8) == 7uz);
    STATIC_REQUIRE(chebyshev_distance(a8, a1) == 7uz);

    STATIC_REQUIRE(chebyshev_distance(a1, h1) == 7uz);
    STATIC_REQUIRE(chebyshev_distance(h1, a1) == 7uz);

    STATIC_REQUIRE(chebyshev_distance(a1, h8) == 7uz);
    STATIC_REQUIRE(chebyshev_distance(h8, a1) == 7uz);

    STATIC_REQUIRE(chebyshev_distance(a8, h1) == 7uz);
    STATIC_REQUIRE(chebyshev_distance(h1, a8) == 7uz);

    STATIC_REQUIRE(chebyshev_distance(a8, h8) == 7uz);
    STATIC_REQUIRE(chebyshev_distance(h8, a8) == 7uz);
}

TEST_CASE("Knight distance", TAGS)
{
    namespace bitboard_masks = chess::board::masks;

    SECTION("From F5")
    {
        static constexpr Square starting { File::F, Rank::Five };

        // clang-format off
        static constexpr std::array distances {
        //  A1
            3, 4, 3, 2, 3, 2, 3, 2,
            4, 3, 2, 3, 2, 3, 2, 3,
            3, 2, 3, 4, 1, 2, 1, 4,
            4, 3, 2, 1, 2, 3, 2, 1,
            3, 2, 3, 2, 3, 0, 3, 2,
            4, 3, 2, 1, 2, 3, 2, 1,
            3, 2, 3, 4, 1, 2, 1, 4,
            4, 3, 2, 3, 2, 3, 2, 3
        //                       H8
        };
        // clang-format on

        for (const auto square : bitboard_masks::ALL.squares())
            REQUIRE(knight_distance(starting, square) == distances[square.index()]);
    }

    SECTION("From G1")
    {
        static constexpr Square starting { File::G, Rank::One };

        // clang-format off
        static constexpr std::array distances {
        //  A1
            4, 3, 2, 3, 2, 3, 0, 3,
            3, 4, 3, 2, 1, 2, 3, 2,
            4, 3, 2, 3, 4, 1, 2, 1,
            3, 4, 3, 2, 3, 2, 3, 2,
            4, 3, 4, 3, 2, 3, 2, 3,
            5, 4, 3, 4, 3, 4, 3, 4,
            4, 5, 4, 3, 4, 3, 4, 3,
            5, 4, 5, 4, 5, 4, 5, 4
        //                       H8
        };
        // clang-format on

        for (const auto square : bitboard_masks::ALL.squares())
            REQUIRE(knight_distance(starting, square) == distances[square.index()]);
    }

    SECTION("From E6")
    {
        static constexpr Square starting { File::E, Rank::Six };

        // clang-format off
        static constexpr std::array distances {
        //  A1
            3, 4, 3, 4, 3, 4, 3, 4,
            4, 3, 2, 3, 2, 3, 2, 3,
            3, 2, 3, 2, 3, 2, 3, 2,
            2, 3, 4, 1, 2, 1, 4, 3,
            3, 2, 1, 2, 3, 2, 1, 2,
            2, 3, 2, 3, 0, 3, 2, 3,
            3, 2, 1, 2, 3, 2, 1, 2,
            2, 3, 4, 1, 2, 1, 4, 3
        //                       H8
        };
        // clang-format on

        for (const auto square : bitboard_masks::ALL.squares())
            REQUIRE(knight_distance(starting, square) == distances[square.index()]);
    }

    SECTION("From D4")
    {
        static constexpr Square starting { File::D, Rank::Four };

        // clang-format off
        static constexpr std::array distances {
        //  A1
            2, 3, 2, 3, 2, 3, 2, 3,
            3, 4, 1, 2, 1, 4, 3, 2,
            2, 1, 2, 3, 2, 1, 2, 3,
            3, 2, 3, 0, 3, 2, 3, 2,
            2, 1, 2, 3, 2, 1, 2, 3,
            3, 4, 1, 2, 1, 4, 3, 2,
            2, 3, 2, 3, 2, 3, 2, 3,
            3, 2, 3, 2, 3, 2, 3, 4
        //                       H8
        };
        // clang-format on

        for (const auto square : bitboard_masks::ALL.squares())
            REQUIRE(knight_distance(starting, square) == distances[square.index()]);
    }
}
