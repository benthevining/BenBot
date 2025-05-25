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
#include <ranges>
#include <utility>
#include <vector>

static constexpr auto TAGS { "[board][Bitboard]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

using chess::board::are_on_same_diagonal;
using chess::board::file_distance;
using chess::board::rank_distance;

namespace bitboard_masks = chess::board::masks;

TEST_CASE("Bitboard - empty", TAGS)
{
    static constexpr Bitboard empty;

    STATIC_REQUIRE(! empty.any());
    STATIC_REQUIRE(empty.none());
    STATIC_REQUIRE(empty.count() == 0uz);
    STATIC_REQUIRE(std::cmp_equal(empty.to_int(), 0));
    STATIC_REQUIRE(std::ranges::empty(empty.squares()));
}

[[nodiscard]] static constexpr auto get_squares(const Bitboard& board)
{
    return board.squares() | std::ranges::to<std::vector>();
}

TEST_CASE("Bitboard - dark/light square masks", TAGS)
{
    SECTION("Dark squares")
    {
        static constexpr auto darkSquares = bitboard_masks::dark_squares();

        STATIC_REQUIRE(darkSquares.count() == 32uz);
        STATIC_REQUIRE(get_squares(darkSquares).size() == darkSquares.count());

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                REQUIRE(darkSquares.test(square) == square.is_dark());
            }
        }

        for (const auto square : darkSquares.squares())
            REQUIRE(square.is_dark());
    }

    SECTION("Light squares")
    {
        static constexpr auto lightSquares = bitboard_masks::light_squares();

        STATIC_REQUIRE(lightSquares.count() == 32uz);
        STATIC_REQUIRE(get_squares(lightSquares).size() == lightSquares.count());

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                REQUIRE(lightSquares.test(square) == square.is_light());
            }
        }

        for (const auto square : lightSquares.squares())
            REQUIRE(square.is_light());
    }
}

TEST_CASE("Bitboard - file masks", TAGS)
{
    SECTION("A file")
    {
        static constexpr auto board = bitboard_masks::a_file();

        STATIC_REQUIRE(board.count() == 8uz);
        STATIC_REQUIRE(get_squares(board).size() == board.count());

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isAFile = file == File::A;

                REQUIRE(board.test(square) == isAFile);
            }
        }

        for (const auto square : board.squares()) {
            REQUIRE(square.file == File::A);

            for (const auto sq2 : board.squares()) {
                REQUIRE(file_distance(square, sq2) == 0uz);

                if (square != sq2)
                    REQUIRE(! are_on_same_diagonal(square, sq2));
            }
        }
    }

    SECTION("H file")
    {
        static constexpr auto board = bitboard_masks::h_file();

        STATIC_REQUIRE(board.count() == 8uz);
        STATIC_REQUIRE(get_squares(board).size() == board.count());

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isHFile = file == File::H;

                REQUIRE(board.test(square) == isHFile);
            }
        }

        for (const auto square : board.squares()) {
            REQUIRE(square.file == File::H);

            for (const auto sq2 : board.squares()) {
                REQUIRE(file_distance(square, sq2) == 0uz);

                if (square != sq2)
                    REQUIRE(! are_on_same_diagonal(square, sq2));
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
        STATIC_REQUIRE(get_squares(board).size() == board.count());

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isRank1 = rank == Rank::One;

                REQUIRE(board.test(square) == isRank1);
            }
        }

        for (const auto square : board.squares()) {
            REQUIRE(square.rank == Rank::One);

            for (const auto sq2 : board.squares()) {
                REQUIRE(rank_distance(square, sq2) == 0uz);

                if (square != sq2)
                    REQUIRE(! are_on_same_diagonal(square, sq2));
            }
        }
    }

    SECTION("Rank 8")
    {
        static constexpr auto board = bitboard_masks::rank_8();

        STATIC_REQUIRE(board.count() == 8uz);
        STATIC_REQUIRE(get_squares(board).size() == board.count());

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            for (const auto file : magic_enum::enum_values<File>()) {
                const Square square { file, rank };

                const bool isRank8 = rank == Rank::Eight;

                REQUIRE(board.test(square) == isRank8);
            }
        }

        for (const auto square : board.squares()) {
            REQUIRE(square.rank == Rank::Eight);

            for (const auto sq2 : board.squares()) {
                REQUIRE(rank_distance(square, sq2) == 0uz);

                if (square != sq2)
                    REQUIRE(! are_on_same_diagonal(square, sq2));
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

        for (const auto [sq1, sq2] : std::views::zip(diagonal.squares(), diagonal.squares()))
            REQUIRE(are_on_same_diagonal(sq1, sq2));

        static const auto squares = get_squares(diagonal);

        for (auto idx = 0uz; idx < diagonal.count(); ++idx) {
            REQUIRE(file_distance(
                        squares.front(), squares[idx])
                    == idx);

            REQUIRE(rank_distance(
                        squares.front(), squares[idx])
                    == idx);
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

        for (const auto [sq1, sq2] : std::views::zip(diagonal.squares(), diagonal.squares()))
            REQUIRE(are_on_same_diagonal(sq1, sq2));

        static const auto squares = get_squares(diagonal);

        for (auto idx = 0uz; idx < diagonal.count(); ++idx) {
            REQUIRE(file_distance(
                        squares.front(), squares[idx])
                    == idx);

            REQUIRE(rank_distance(
                        squares.front(), squares[idx])
                    == idx);
        }
    }
}
