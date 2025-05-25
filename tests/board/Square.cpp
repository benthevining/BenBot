/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <format>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <magic_enum/magic_enum.hpp>

static constexpr auto TAGS { "[board][Square]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

namespace gen = Catch::Generators;

TEST_CASE("Square - to/from index", TAGS)
{
#define SQUARE_TO_FROM_IDX(name, file, rank, idx)          \
    SECTION(name)                                          \
    {                                                      \
        static constexpr Square square { file, rank };     \
        STATIC_REQUIRE(square.index() == idx);             \
        STATIC_REQUIRE(square == Square::from_index(idx)); \
    }

    SQUARE_TO_FROM_IDX("A1", File::A, Rank::One, 0uz);
    SQUARE_TO_FROM_IDX("B1", File::B, Rank::One, 1uz);
    SQUARE_TO_FROM_IDX("C1", File::C, Rank::One, 2uz);
    SQUARE_TO_FROM_IDX("D1", File::D, Rank::One, 3uz);
    SQUARE_TO_FROM_IDX("E1", File::E, Rank::One, 4uz);
    SQUARE_TO_FROM_IDX("F1", File::F, Rank::One, 5uz);
    SQUARE_TO_FROM_IDX("G1", File::G, Rank::One, 6uz);
    SQUARE_TO_FROM_IDX("H1", File::H, Rank::One, 7uz);

    SQUARE_TO_FROM_IDX("A2", File::A, Rank::Two, 8uz);
    SQUARE_TO_FROM_IDX("B2", File::B, Rank::Two, 9uz);
    SQUARE_TO_FROM_IDX("C2", File::C, Rank::Two, 10uz);
    SQUARE_TO_FROM_IDX("D2", File::D, Rank::Two, 11uz);
    SQUARE_TO_FROM_IDX("E2", File::E, Rank::Two, 12uz);
    SQUARE_TO_FROM_IDX("F2", File::F, Rank::Two, 13uz);
    SQUARE_TO_FROM_IDX("G2", File::G, Rank::Two, 14uz);
    SQUARE_TO_FROM_IDX("H2", File::H, Rank::Two, 15uz);

    SQUARE_TO_FROM_IDX("A3", File::A, Rank::Three, 16uz);
    SQUARE_TO_FROM_IDX("B3", File::B, Rank::Three, 17uz);
    SQUARE_TO_FROM_IDX("C3", File::C, Rank::Three, 18uz);
    SQUARE_TO_FROM_IDX("D3", File::D, Rank::Three, 19uz);
    SQUARE_TO_FROM_IDX("E3", File::E, Rank::Three, 20uz);
    SQUARE_TO_FROM_IDX("F3", File::F, Rank::Three, 21uz);
    SQUARE_TO_FROM_IDX("G3", File::G, Rank::Three, 22uz);
    SQUARE_TO_FROM_IDX("H3", File::H, Rank::Three, 23uz);

    SQUARE_TO_FROM_IDX("A4", File::A, Rank::Four, 24uz);
    SQUARE_TO_FROM_IDX("B4", File::B, Rank::Four, 25uz);
    SQUARE_TO_FROM_IDX("C4", File::C, Rank::Four, 26uz);
    SQUARE_TO_FROM_IDX("D4", File::D, Rank::Four, 27uz);
    SQUARE_TO_FROM_IDX("E4", File::E, Rank::Four, 28uz);
    SQUARE_TO_FROM_IDX("F4", File::F, Rank::Four, 29uz);
    SQUARE_TO_FROM_IDX("G4", File::G, Rank::Four, 30uz);
    SQUARE_TO_FROM_IDX("H4", File::H, Rank::Four, 31uz);

    SQUARE_TO_FROM_IDX("A5", File::A, Rank::Five, 32uz);
    SQUARE_TO_FROM_IDX("B5", File::B, Rank::Five, 33uz);
    SQUARE_TO_FROM_IDX("C5", File::C, Rank::Five, 34uz);
    SQUARE_TO_FROM_IDX("D5", File::D, Rank::Five, 35uz);
    SQUARE_TO_FROM_IDX("E5", File::E, Rank::Five, 36uz);
    SQUARE_TO_FROM_IDX("F5", File::F, Rank::Five, 37uz);
    SQUARE_TO_FROM_IDX("G5", File::G, Rank::Five, 38uz);
    SQUARE_TO_FROM_IDX("H5", File::H, Rank::Five, 39uz);

    SQUARE_TO_FROM_IDX("A6", File::A, Rank::Six, 40uz);
    SQUARE_TO_FROM_IDX("B6", File::B, Rank::Six, 41uz);
    SQUARE_TO_FROM_IDX("C6", File::C, Rank::Six, 42uz);
    SQUARE_TO_FROM_IDX("D6", File::D, Rank::Six, 43uz);
    SQUARE_TO_FROM_IDX("E6", File::E, Rank::Six, 44uz);
    SQUARE_TO_FROM_IDX("F6", File::F, Rank::Six, 45uz);
    SQUARE_TO_FROM_IDX("G6", File::G, Rank::Six, 46uz);
    SQUARE_TO_FROM_IDX("H6", File::H, Rank::Six, 47uz);

    SQUARE_TO_FROM_IDX("A7", File::A, Rank::Seven, 48uz);
    SQUARE_TO_FROM_IDX("B7", File::B, Rank::Seven, 49uz);
    SQUARE_TO_FROM_IDX("C7", File::C, Rank::Seven, 50uz);
    SQUARE_TO_FROM_IDX("D7", File::D, Rank::Seven, 51uz);
    SQUARE_TO_FROM_IDX("E7", File::E, Rank::Seven, 52uz);
    SQUARE_TO_FROM_IDX("F7", File::F, Rank::Seven, 53uz);
    SQUARE_TO_FROM_IDX("G7", File::G, Rank::Seven, 54uz);
    SQUARE_TO_FROM_IDX("H7", File::H, Rank::Seven, 55uz);

    SQUARE_TO_FROM_IDX("A8", File::A, Rank::Eight, 56uz);
    SQUARE_TO_FROM_IDX("B8", File::B, Rank::Eight, 57uz);
    SQUARE_TO_FROM_IDX("C8", File::C, Rank::Eight, 58uz);
    SQUARE_TO_FROM_IDX("D8", File::D, Rank::Eight, 59uz);
    SQUARE_TO_FROM_IDX("E8", File::E, Rank::Eight, 60uz);
    SQUARE_TO_FROM_IDX("F8", File::F, Rank::Eight, 61uz);
    SQUARE_TO_FROM_IDX("G8", File::G, Rank::Eight, 62uz);
    SQUARE_TO_FROM_IDX("H8", File::H, Rank::Eight, 63uz);

#undef SQUARE_TO_FROM_IDX
}

TEST_CASE("Square - is_light()/is_dark()", TAGS)
{
    STATIC_REQUIRE(Square { File::A, Rank::One }.is_dark());
    STATIC_REQUIRE(Square { File::B, Rank::One }.is_light());
    STATIC_REQUIRE(Square { File::C, Rank::One }.is_dark());
    STATIC_REQUIRE(Square { File::D, Rank::One }.is_light());
    STATIC_REQUIRE(Square { File::E, Rank::One }.is_dark());
    STATIC_REQUIRE(Square { File::F, Rank::One }.is_light());
    STATIC_REQUIRE(Square { File::G, Rank::One }.is_dark());
    STATIC_REQUIRE(Square { File::H, Rank::One }.is_light());

    STATIC_REQUIRE(Square { File::A, Rank::Two }.is_light());
    STATIC_REQUIRE(Square { File::B, Rank::Two }.is_dark());
    STATIC_REQUIRE(Square { File::C, Rank::Two }.is_light());
    STATIC_REQUIRE(Square { File::D, Rank::Two }.is_dark());
    STATIC_REQUIRE(Square { File::E, Rank::Two }.is_light());
    STATIC_REQUIRE(Square { File::F, Rank::Two }.is_dark());
    STATIC_REQUIRE(Square { File::G, Rank::Two }.is_light());
    STATIC_REQUIRE(Square { File::H, Rank::Two }.is_dark());

    STATIC_REQUIRE(Square { File::A, Rank::Three }.is_dark());
    STATIC_REQUIRE(Square { File::B, Rank::Three }.is_light());
    STATIC_REQUIRE(Square { File::C, Rank::Three }.is_dark());
    STATIC_REQUIRE(Square { File::D, Rank::Three }.is_light());
    STATIC_REQUIRE(Square { File::E, Rank::Three }.is_dark());
    STATIC_REQUIRE(Square { File::F, Rank::Three }.is_light());
    STATIC_REQUIRE(Square { File::G, Rank::Three }.is_dark());
    STATIC_REQUIRE(Square { File::H, Rank::Three }.is_light());

    STATIC_REQUIRE(Square { File::A, Rank::Four }.is_light());
    STATIC_REQUIRE(Square { File::B, Rank::Four }.is_dark());
    STATIC_REQUIRE(Square { File::C, Rank::Four }.is_light());
    STATIC_REQUIRE(Square { File::D, Rank::Four }.is_dark());
    STATIC_REQUIRE(Square { File::E, Rank::Four }.is_light());
    STATIC_REQUIRE(Square { File::F, Rank::Four }.is_dark());
    STATIC_REQUIRE(Square { File::G, Rank::Four }.is_light());
    STATIC_REQUIRE(Square { File::H, Rank::Four }.is_dark());

    STATIC_REQUIRE(Square { File::A, Rank::Five }.is_dark());
    STATIC_REQUIRE(Square { File::B, Rank::Five }.is_light());
    STATIC_REQUIRE(Square { File::C, Rank::Five }.is_dark());
    STATIC_REQUIRE(Square { File::D, Rank::Five }.is_light());
    STATIC_REQUIRE(Square { File::E, Rank::Five }.is_dark());
    STATIC_REQUIRE(Square { File::F, Rank::Five }.is_light());
    STATIC_REQUIRE(Square { File::G, Rank::Five }.is_dark());
    STATIC_REQUIRE(Square { File::H, Rank::Five }.is_light());

    STATIC_REQUIRE(Square { File::A, Rank::Six }.is_light());
    STATIC_REQUIRE(Square { File::B, Rank::Six }.is_dark());
    STATIC_REQUIRE(Square { File::C, Rank::Six }.is_light());
    STATIC_REQUIRE(Square { File::D, Rank::Six }.is_dark());
    STATIC_REQUIRE(Square { File::E, Rank::Six }.is_light());
    STATIC_REQUIRE(Square { File::F, Rank::Six }.is_dark());
    STATIC_REQUIRE(Square { File::G, Rank::Six }.is_light());
    STATIC_REQUIRE(Square { File::H, Rank::Six }.is_dark());

    STATIC_REQUIRE(Square { File::A, Rank::Seven }.is_dark());
    STATIC_REQUIRE(Square { File::B, Rank::Seven }.is_light());
    STATIC_REQUIRE(Square { File::C, Rank::Seven }.is_dark());
    STATIC_REQUIRE(Square { File::D, Rank::Seven }.is_light());
    STATIC_REQUIRE(Square { File::E, Rank::Seven }.is_dark());
    STATIC_REQUIRE(Square { File::F, Rank::Seven }.is_light());
    STATIC_REQUIRE(Square { File::G, Rank::Seven }.is_dark());
    STATIC_REQUIRE(Square { File::H, Rank::Seven }.is_light());

    STATIC_REQUIRE(Square { File::A, Rank::Eight }.is_light());
    STATIC_REQUIRE(Square { File::B, Rank::Eight }.is_dark());
    STATIC_REQUIRE(Square { File::C, Rank::Eight }.is_light());
    STATIC_REQUIRE(Square { File::D, Rank::Eight }.is_dark());
    STATIC_REQUIRE(Square { File::E, Rank::Eight }.is_light());
    STATIC_REQUIRE(Square { File::F, Rank::Eight }.is_dark());
    STATIC_REQUIRE(Square { File::G, Rank::Eight }.is_light());
    STATIC_REQUIRE(Square { File::H, Rank::Eight }.is_dark());
}

TEST_CASE("Square - to/from string", TAGS)
{
#define SQUARE_TO_FROM_STRING(str, strLower, file, rank)         \
    SECTION(str)                                                 \
    {                                                            \
        static constexpr Square square { file, rank };           \
        STATIC_REQUIRE(square == Square::from_string(str));      \
        STATIC_REQUIRE(square == Square::from_string(strLower)); \
        REQUIRE(std::format("{}", square) == str);               \
    }

    SQUARE_TO_FROM_STRING("A1", "a1", File::A, Rank::One);
    SQUARE_TO_FROM_STRING("B1", "b1", File::B, Rank::One);
    SQUARE_TO_FROM_STRING("C1", "c1", File::C, Rank::One);
    SQUARE_TO_FROM_STRING("D1", "d1", File::D, Rank::One);
    SQUARE_TO_FROM_STRING("E1", "e1", File::E, Rank::One);
    SQUARE_TO_FROM_STRING("F1", "f1", File::F, Rank::One);
    SQUARE_TO_FROM_STRING("G1", "g1", File::G, Rank::One);
    SQUARE_TO_FROM_STRING("H1", "h1", File::H, Rank::One);

    SQUARE_TO_FROM_STRING("A2", "a2", File::A, Rank::Two);
    SQUARE_TO_FROM_STRING("B2", "b2", File::B, Rank::Two);
    SQUARE_TO_FROM_STRING("C2", "c2", File::C, Rank::Two);
    SQUARE_TO_FROM_STRING("D2", "d2", File::D, Rank::Two);
    SQUARE_TO_FROM_STRING("E2", "e2", File::E, Rank::Two);
    SQUARE_TO_FROM_STRING("F2", "f2", File::F, Rank::Two);
    SQUARE_TO_FROM_STRING("G2", "g2", File::G, Rank::Two);
    SQUARE_TO_FROM_STRING("H2", "h2", File::H, Rank::Two);

    SQUARE_TO_FROM_STRING("A3", "a3", File::A, Rank::Three);
    SQUARE_TO_FROM_STRING("B3", "b3", File::B, Rank::Three);
    SQUARE_TO_FROM_STRING("C3", "c3", File::C, Rank::Three);
    SQUARE_TO_FROM_STRING("D3", "d3", File::D, Rank::Three);
    SQUARE_TO_FROM_STRING("E3", "e3", File::E, Rank::Three);
    SQUARE_TO_FROM_STRING("F3", "f3", File::F, Rank::Three);
    SQUARE_TO_FROM_STRING("G3", "g3", File::G, Rank::Three);
    SQUARE_TO_FROM_STRING("H3", "h3", File::H, Rank::Three);

    SQUARE_TO_FROM_STRING("A4", "a4", File::A, Rank::Four);
    SQUARE_TO_FROM_STRING("B4", "b4", File::B, Rank::Four);
    SQUARE_TO_FROM_STRING("C4", "c4", File::C, Rank::Four);
    SQUARE_TO_FROM_STRING("D4", "d4", File::D, Rank::Four);
    SQUARE_TO_FROM_STRING("E4", "e4", File::E, Rank::Four);
    SQUARE_TO_FROM_STRING("F4", "f4", File::F, Rank::Four);
    SQUARE_TO_FROM_STRING("G4", "g4", File::G, Rank::Four);
    SQUARE_TO_FROM_STRING("H4", "h4", File::H, Rank::Four);

    SQUARE_TO_FROM_STRING("A5", "a5", File::A, Rank::Five);
    SQUARE_TO_FROM_STRING("B5", "b5", File::B, Rank::Five);
    SQUARE_TO_FROM_STRING("C5", "c5", File::C, Rank::Five);
    SQUARE_TO_FROM_STRING("D5", "d5", File::D, Rank::Five);
    SQUARE_TO_FROM_STRING("E5", "e5", File::E, Rank::Five);
    SQUARE_TO_FROM_STRING("F5", "f5", File::F, Rank::Five);
    SQUARE_TO_FROM_STRING("G5", "g5", File::G, Rank::Five);
    SQUARE_TO_FROM_STRING("H5", "h5", File::H, Rank::Five);

    SQUARE_TO_FROM_STRING("A6", "a6", File::A, Rank::Six);
    SQUARE_TO_FROM_STRING("B6", "b6", File::B, Rank::Six);
    SQUARE_TO_FROM_STRING("C6", "c6", File::C, Rank::Six);
    SQUARE_TO_FROM_STRING("D6", "d6", File::D, Rank::Six);
    SQUARE_TO_FROM_STRING("E6", "e6", File::E, Rank::Six);
    SQUARE_TO_FROM_STRING("F6", "f6", File::F, Rank::Six);
    SQUARE_TO_FROM_STRING("G6", "g6", File::G, Rank::Six);
    SQUARE_TO_FROM_STRING("H6", "h6", File::H, Rank::Six);

    SQUARE_TO_FROM_STRING("A7", "a7", File::A, Rank::Seven);
    SQUARE_TO_FROM_STRING("B7", "b7", File::B, Rank::Seven);
    SQUARE_TO_FROM_STRING("C7", "c7", File::C, Rank::Seven);
    SQUARE_TO_FROM_STRING("D7", "d7", File::D, Rank::Seven);
    SQUARE_TO_FROM_STRING("E7", "e7", File::E, Rank::Seven);
    SQUARE_TO_FROM_STRING("F7", "f7", File::F, Rank::Seven);
    SQUARE_TO_FROM_STRING("G7", "g7", File::G, Rank::Seven);
    SQUARE_TO_FROM_STRING("H7", "h7", File::H, Rank::Seven);

    SQUARE_TO_FROM_STRING("A8", "a8", File::A, Rank::Eight);
    SQUARE_TO_FROM_STRING("B8", "b8", File::B, Rank::Eight);
    SQUARE_TO_FROM_STRING("C8", "c8", File::C, Rank::Eight);
    SQUARE_TO_FROM_STRING("D8", "d8", File::D, Rank::Eight);
    SQUARE_TO_FROM_STRING("E8", "e8", File::E, Rank::Eight);
    SQUARE_TO_FROM_STRING("F8", "f8", File::F, Rank::Eight);
    SQUARE_TO_FROM_STRING("G8", "g8", File::G, Rank::Eight);
    SQUARE_TO_FROM_STRING("H8", "h8", File::H, Rank::Eight);

#undef SQUARE_TO_FROM_STRING
}

TEST_CASE("Square - is_white_territory()/is_black_territory()", TAGS)
{
    SECTION("is_white_territory()")
    {
        const auto index = GENERATE(gen::range(0uz, 32uz));

        const auto square = Square::from_index(index);

        REQUIRE(square.is_white_territory());
    }

    SECTION("is_black_territory()")
    {
        const auto index = GENERATE(gen::range(32uz, 64uz));

        const auto square = Square::from_index(index);

        REQUIRE(square.is_black_territory());
    }
}

TEST_CASE("Square - is_queenside()/is_kingside()", TAGS)
{
    SECTION("Queenside")
    {
        const auto file = static_cast<File>(
            GENERATE(gen::range(0uz, 4uz)));

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            const Square square { file, rank };

            REQUIRE(square.is_queenside());
            REQUIRE(! square.is_kingside());
        }
    }

    SECTION("Kingside")
    {
        const auto file = static_cast<File>(
            GENERATE(gen::range(4uz, 8uz)));

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            const Square square { file, rank };

            REQUIRE(square.is_kingside());
            REQUIRE(! square.is_queenside());
        }
    }
}

TEST_CASE("File distance", TAGS)
{
    using chess::board::file_distance;

    SECTION("A/B files")
    {
        for (const auto rank : magic_enum::enum_values<Rank>()) {
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
        for (const auto rank : magic_enum::enum_values<Rank>()) {
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
        for (const auto rank : magic_enum::enum_values<Rank>()) {
            const Square aFile { File::A, rank };
            const Square hFile { File::H, rank };

            REQUIRE(file_distance(aFile, hFile) == 7uz);
            REQUIRE(file_distance(hFile, aFile) == 7uz);
        }
    }
}

TEST_CASE("Rank distance", TAGS)
{
    using chess::board::rank_distance;

    SECTION("1/2")
    {
        for (const auto file : magic_enum::enum_values<File>()) {
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
        for (const auto file : magic_enum::enum_values<File>()) {
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
        for (const auto file : magic_enum::enum_values<File>()) {
            const Square rank1 { file, Rank::One };
            const Square rank8 { file, Rank::Eight };

            REQUIRE(rank_distance(rank1, rank8) == 7uz);
            REQUIRE(rank_distance(rank8, rank1) == 7uz);
        }
    }
}

TEST_CASE("Manhattan distance", TAGS)
{
    using chess::board::manhattan_distance;

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

TEST_CASE("Chebyshev distance", TAGS)
{
    using chess::board::chebyshev_distance;

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
    using chess::board::knight_distance;

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

        for (const auto square : bitboard_masks::all().squares())
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

        for (const auto square : bitboard_masks::all().squares())
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

        for (const auto square : bitboard_masks::all().squares())
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

        for (const auto square : bitboard_masks::all().squares()) {
            INFO(std::format("Square: {} (index {})", square, square.index()));
            REQUIRE(knight_distance(starting, square) == distances[square.index()]);
        }
    }
}
