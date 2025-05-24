/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Square.hpp>
#include <magic_enum/magic_enum.hpp>
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
}

TEST_CASE("Bitboard - dark/light square masks", TAGS)
{
    SECTION("Dark squares")
    {
        static constexpr auto darkSquares = bitboard_masks::dark_squares();

        STATIC_REQUIRE(darkSquares.count() == 32uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                REQUIRE(darkSquares.test(square) == square.is_dark());
            }
        }
    }

    SECTION("Light squares")
    {
        static constexpr auto lightSquares = bitboard_masks::light_squares();

        STATIC_REQUIRE(lightSquares.count() == 32uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                REQUIRE(lightSquares.test(square) == square.is_light());
            }
        }
    }
}

TEST_CASE("Bitboard - file masks", TAGS)
{
    SECTION("A file")
    {
        static constexpr auto board = bitboard_masks::a_file();

        STATIC_REQUIRE(board.count() == 8uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isAFile = file == File::A;

                REQUIRE(board.test(square) == isAFile);
            }
        }
    }

    SECTION("H file")
    {
        static constexpr auto board = bitboard_masks::h_file();

        STATIC_REQUIRE(board.count() == 8uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isHFile = file == File::H;

                REQUIRE(board.test(square) == isHFile);
            }
        }
    }
}

TEST_CASE("Bitboard - rank masks", TAGS)
{
    SECTION("Rank 1")
    {
        static constexpr auto board = bitboard_masks::rank_1();

        STATIC_REQUIRE(board.count() == 8uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isRank1 = rank == Rank::One;

                REQUIRE(board.test(square) == isRank1);
            }
        }
    }

    SECTION("Rank 8")
    {
        static constexpr auto board = bitboard_masks::rank_8();

        STATIC_REQUIRE(board.count() == 8uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isRank8 = rank == Rank::Eight;

                REQUIRE(board.test(square) == isRank8);
            }
        }
    }
}

TEST_CASE("Bitboard - diagonal masks", TAGS)
{
    SECTION("A1-H8")
    {
        static constexpr auto diagonal = bitboard_masks::a1_h8_diagonal();

        STATIC_REQUIRE(diagonal.count() == 8uz);

        // there should only be 1 bit set in each rank & file

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            REQUIRE(std::ranges::count_if(
                        magic_enum::enum_values<File>(),
                        [rank](File file) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }

        for (const auto file : magic_enum::enum_values<File>()) {
            REQUIRE(std::ranges::count_if(
                        magic_enum::enum_values<Rank>(),
                        [file](Rank rank) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }
    }

    SECTION("A8-H1")
    {
        static constexpr auto diagonal = bitboard_masks::a8_h1_diagonal();

        STATIC_REQUIRE(diagonal.count() == 8uz);

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            REQUIRE(std::ranges::count_if(
                        magic_enum::enum_values<File>(),
                        [rank](File file) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }

        for (const auto file : magic_enum::enum_values<File>()) {
            REQUIRE(std::ranges::count_if(
                        magic_enum::enum_values<Rank>(),
                        [file](Rank rank) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }
    }
}
