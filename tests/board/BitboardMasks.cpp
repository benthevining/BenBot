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
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Square.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <vector>

static constexpr auto TAGS { "[board][Bitboard][masks]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

using chess::board::file_distance;
using chess::board::rank_distance;

namespace bitboard_masks = chess::board::masks;

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
#define TEST_FILE_MASK(str, correctFile, mask)                        \
    SECTION(str)                                                      \
    {                                                                 \
        static constexpr auto board = mask;                           \
                                                                      \
        STATIC_REQUIRE(board.count() == 8uz);                         \
        STATIC_REQUIRE(get_squares(board).size() == board.count());   \
                                                                      \
        for (const auto rank : magic_enum::enum_values<Rank>()) {     \
            for (const auto file : magic_enum::enum_values<File>()) { \
                const Square square { file, rank };                   \
                                                                      \
                const bool isCorrectFile = file == correctFile;       \
                                                                      \
                REQUIRE(board.test(square) == isCorrectFile);         \
            }                                                         \
        }                                                             \
                                                                      \
        for (const auto square : board.squares()) {                   \
            REQUIRE(square.file == correctFile);                      \
                                                                      \
            for (const auto sq2 : board.squares()) {                  \
                REQUIRE(file_distance(square, sq2) == 0uz);           \
                                                                      \
                if (square != sq2)                                    \
                    REQUIRE(! are_on_same_diagonal(square, sq2));     \
            }                                                         \
        }                                                             \
    }

    TEST_FILE_MASK("A file", File::A, bitboard_masks::files::a());
    TEST_FILE_MASK("B file", File::B, bitboard_masks::files::b());
    TEST_FILE_MASK("C file", File::C, bitboard_masks::files::c());
    TEST_FILE_MASK("D file", File::D, bitboard_masks::files::d());
    TEST_FILE_MASK("E file", File::E, bitboard_masks::files::e());
    TEST_FILE_MASK("F file", File::F, bitboard_masks::files::f());
    TEST_FILE_MASK("G file", File::G, bitboard_masks::files::g());
    TEST_FILE_MASK("H file", File::H, bitboard_masks::files::h());

#undef TEST_FILE_MASK
}

TEST_CASE("Bitboard - rank masks", TAGS)
{
#define TEST_RANK_MASK(str, correctRank, mask)                        \
    SECTION(str)                                                      \
    {                                                                 \
        static constexpr auto board = mask;                           \
                                                                      \
        STATIC_REQUIRE(board.count() == 8uz);                         \
        STATIC_REQUIRE(get_squares(board).size() == board.count());   \
                                                                      \
        for (const auto rank : magic_enum::enum_values<Rank>()) {     \
            for (const auto file : magic_enum::enum_values<File>()) { \
                const Square square { file, rank };                   \
                                                                      \
                const bool isCorrectRank = rank == correctRank;       \
                                                                      \
                REQUIRE(board.test(square) == isCorrectRank);         \
            }                                                         \
        }                                                             \
                                                                      \
        for (const auto square : board.squares()) {                   \
            REQUIRE(square.rank == correctRank);                      \
                                                                      \
            for (const auto sq2 : board.squares()) {                  \
                REQUIRE(rank_distance(square, sq2) == 0uz);           \
                                                                      \
                if (square != sq2)                                    \
                    REQUIRE(! are_on_same_diagonal(square, sq2));     \
            }                                                         \
        }                                                             \
    }

    TEST_RANK_MASK("Rank 1", Rank::One, bitboard_masks::ranks::one());
    TEST_RANK_MASK("Rank 2", Rank::Two, bitboard_masks::ranks::two());
    TEST_RANK_MASK("Rank 3", Rank::Three, bitboard_masks::ranks::three());
    TEST_RANK_MASK("Rank 4", Rank::Four, bitboard_masks::ranks::four());
    TEST_RANK_MASK("Rank 5", Rank::Five, bitboard_masks::ranks::five());
    TEST_RANK_MASK("Rank 6", Rank::Six, bitboard_masks::ranks::six());
    TEST_RANK_MASK("Rank 7", Rank::Seven, bitboard_masks::ranks::seven());
    TEST_RANK_MASK("Rank 8", Rank::Eight, bitboard_masks::ranks::eight());

#undef TEST_RANK_MASK
}

TEST_CASE("Bitboard - diagonal masks", TAGS)
{
    using chess::board::are_on_same_diagonal;

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

TEST_CASE("Starting position masks", TAGS)
{
    SECTION("White")
    {
        SECTION("Pawns")
        {
            static constexpr auto pos = bitboard_masks::starting::white::pawns();

            STATIC_REQUIRE(pos.count() == 8uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::Two);
            }
        }

        SECTION("Rooks")
        {
            static constexpr auto pos = bitboard_masks::starting::white::rooks();

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(((square.file == File::A) || (square.file == File::H)));
            }
        }

        SECTION("Knights")
        {
            static constexpr auto pos = bitboard_masks::starting::white::knights();

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(((square.file == File::B) || (square.file == File::G)));
            }
        }

        SECTION("Bishops")
        {
            static constexpr auto pos = bitboard_masks::starting::white::bishops();

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(((square.file == File::C) || (square.file == File::F)));
            }
        }

        SECTION("Queen")
        {
            static constexpr auto pos = bitboard_masks::starting::white::queen();

            STATIC_REQUIRE(pos.count() == 1uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(square.file == File::D);
            }
        }

        SECTION("King")
        {
            static constexpr auto pos = bitboard_masks::starting::white::king();

            STATIC_REQUIRE(pos.count() == 1uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(square.file == File::E);
            }
        }
    }

    SECTION("Black")
    {
        SECTION("Pawns")
        {
            static constexpr auto pos = bitboard_masks::starting::black::pawns();

            STATIC_REQUIRE(pos.count() == 8uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Seven);
            }
        }

        SECTION("Rooks")
        {
            static constexpr auto pos = bitboard_masks::starting::black::rooks();

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(((square.file == File::A) || (square.file == File::H)));
            }
        }

        SECTION("Knights")
        {
            static constexpr auto pos = bitboard_masks::starting::black::knights();

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(((square.file == File::B) || (square.file == File::G)));
            }
        }

        SECTION("Bishops")
        {
            static constexpr auto pos = bitboard_masks::starting::black::bishops();

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(((square.file == File::C) || (square.file == File::F)));
            }
        }

        SECTION("Queen")
        {
            static constexpr auto pos = bitboard_masks::starting::black::queen();

            STATIC_REQUIRE(pos.count() == 1uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(square.file == File::D);
            }
        }

        SECTION("King")
        {
            static constexpr auto pos = bitboard_masks::starting::black::king();

            STATIC_REQUIRE(pos.count() == 1uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(square.file == File::E);
            }
        }
    }
}

TEST_CASE("Center mask", TAGS)
{
    static constexpr auto center = bitboard_masks::center();

    STATIC_REQUIRE(center.count() == 4uz);

    for (const auto [file, rank] : center.squares()) {
        REQUIRE(((file == File::D) || (file == File::E)));
        REQUIRE(((rank == Rank::Four) || (rank == Rank::Five)));
    }
}

TEST_CASE("Perimeter mask", TAGS)
{
    static constexpr auto perimeter = bitboard_masks::perimeter();

    STATIC_REQUIRE(perimeter.count() == 28uz);

    for (const auto rank : magic_enum::enum_values<Rank>()) {
        REQUIRE(perimeter.test(Square { File::A, rank }));
        REQUIRE(perimeter.test(Square { File::H, rank }));
    }

    for (const auto file : magic_enum::enum_values<File>()) {
        REQUIRE(perimeter.test(Square { file, Rank::One }));
        REQUIRE(perimeter.test(Square { file, Rank::Eight }));
    }
}
