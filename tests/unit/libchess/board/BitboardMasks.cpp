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

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <string_view>
#include <vector>

static constexpr auto TAGS { "[board][Bitboard][masks]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

using chess::board::file_distance;
using chess::board::rank_distance;

using magic_enum::enum_values;

namespace bitboard_masks = chess::board::masks;

namespace {
[[nodiscard]] constexpr auto get_squares(const Bitboard board)
{
    return board.squares() | std::ranges::to<std::vector>();
}
} // namespace

TEST_CASE("Bitboard - dark/light square masks", TAGS)
{
    SECTION("Dark squares")
    {
        using bitboard_masks::DARK_SQUARES;

        STATIC_REQUIRE(DARK_SQUARES.count() == 32uz);
        STATIC_REQUIRE(get_squares(DARK_SQUARES).size() == DARK_SQUARES.count());

        for (const auto rank : enum_values<Rank>()) {
            for (const auto file : enum_values<File>()) {
                const Square square { file, rank };

                REQUIRE(DARK_SQUARES.test(square) == square.is_dark());
            }
        }

        for (const auto square : DARK_SQUARES.squares())
            REQUIRE(square.is_dark());
    }

    SECTION("Light squares")
    {
        using bitboard_masks::LIGHT_SQUARES;

        STATIC_REQUIRE(LIGHT_SQUARES.count() == 32uz);
        STATIC_REQUIRE(get_squares(LIGHT_SQUARES).size() == LIGHT_SQUARES.count());

        for (const auto rank : enum_values<Rank>()) {
            for (const auto file : enum_values<File>()) {
                const Square square { file, rank };

                REQUIRE(LIGHT_SQUARES.test(square) == square.is_light());
            }
        }

        for (const auto square : LIGHT_SQUARES.squares())
            REQUIRE(square.is_light());
    }
}

namespace {
constexpr void test_file_mask(
    const std::string_view sectionName,
    const File correctFile, const Bitboard mask)
{
    SECTION(sectionName.data())
    {
        REQUIRE(mask.count() == 8uz);
        REQUIRE(get_squares(mask).size() == mask.count());

        for (const auto rank : enum_values<Rank>()) {
            for (const auto file : enum_values<File>()) {
                const Square square { file, rank };

                const bool isCorrectFile = file == correctFile;

                REQUIRE(mask.test(square) == isCorrectFile);
            }
        }

        for (const auto square : mask.squares()) {
            REQUIRE(square.file == correctFile);

            for (const auto sq2 : mask.squares()) {
                REQUIRE(file_distance(square, sq2) == 0uz);

                if (square != sq2)
                    REQUIRE_FALSE(are_on_same_diagonal(square, sq2));
            }
        }
    }
}
} // namespace

TEST_CASE("Bitboard - file masks", TAGS)
{
    test_file_mask("A file", File::A, bitboard_masks::files::A);
    test_file_mask("B file", File::B, bitboard_masks::files::B);
    test_file_mask("C file", File::C, bitboard_masks::files::C);
    test_file_mask("D file", File::D, bitboard_masks::files::D);
    test_file_mask("E file", File::E, bitboard_masks::files::E);
    test_file_mask("F file", File::F, bitboard_masks::files::F);
    test_file_mask("G file", File::G, bitboard_masks::files::G);
    test_file_mask("H file", File::H, bitboard_masks::files::H);
}

namespace {
constexpr void test_rank_mask(
    const std::string_view sectionName,
    const Rank correctRank, const Bitboard mask)
{
    SECTION(sectionName.data())
    {
        REQUIRE(mask.count() == 8uz);
        REQUIRE(get_squares(mask).size() == mask.count());

        for (const auto rank : enum_values<Rank>()) {
            for (const auto file : enum_values<File>()) {
                const Square square { file, rank };

                const bool isCorrectRank = rank == correctRank;

                REQUIRE(mask.test(square) == isCorrectRank);
            }
        }

        for (const auto square : mask.squares()) {
            REQUIRE(square.rank == correctRank);

            for (const auto sq2 : mask.squares()) {
                REQUIRE(rank_distance(square, sq2) == 0uz);

                if (square != sq2)
                    REQUIRE_FALSE(are_on_same_diagonal(square, sq2));
            }
        }
    }
}
} // namespace

TEST_CASE("Bitboard - rank masks", TAGS)
{
    test_rank_mask("Rank 1", Rank::One, bitboard_masks::ranks::ONE);
    test_rank_mask("Rank 2", Rank::Two, bitboard_masks::ranks::TWO);
    test_rank_mask("Rank 3", Rank::Three, bitboard_masks::ranks::THREE);
    test_rank_mask("Rank 4", Rank::Four, bitboard_masks::ranks::FOUR);
    test_rank_mask("Rank 5", Rank::Five, bitboard_masks::ranks::FIVE);
    test_rank_mask("Rank 6", Rank::Six, bitboard_masks::ranks::SIX);
    test_rank_mask("Rank 7", Rank::Seven, bitboard_masks::ranks::SEVEN);
    test_rank_mask("Rank 8", Rank::Eight, bitboard_masks::ranks::EIGHT);
}

TEST_CASE("Bitboard - diagonal masks", TAGS)
{
    using chess::board::are_on_same_diagonal;

    SECTION("A1-H8")
    {
        static constexpr auto diagonal = bitboard_masks::MAIN_DIAGONAL;

        STATIC_REQUIRE(diagonal.count() == 8uz);

        // there should only be 1 bit set in each rank & file

        for (const auto rank : enum_values<Rank>()) {
            REQUIRE(std::ranges::count_if(
                        enum_values<File>(),
                        [rank](File file) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }

        for (const auto file : enum_values<File>()) {
            REQUIRE(std::ranges::count_if(
                        enum_values<Rank>(),
                        [file](Rank rank) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }

        for (const auto [sq1, sq2] : std::views::zip(diagonal.squares(), diagonal.squares()))
            REQUIRE(are_on_same_diagonal(sq1, sq2));

        const auto squares = get_squares(diagonal);

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
        static constexpr auto diagonal = bitboard_masks::MAIN_ANTIDIAGONAL;

        STATIC_REQUIRE(diagonal.count() == 8uz);

        for (const auto rank : enum_values<Rank>()) {
            REQUIRE(std::ranges::count_if(
                        enum_values<File>(),
                        [rank](File file) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }

        for (const auto file : enum_values<File>()) {
            REQUIRE(std::ranges::count_if(
                        enum_values<Rank>(),
                        [file](Rank rank) { return diagonal.test(Square { file, rank }); })
                    == 1uz);
        }

        for (const auto [sq1, sq2] : std::views::zip(diagonal.squares(), diagonal.squares()))
            REQUIRE(are_on_same_diagonal(sq1, sq2));

        const auto squares = get_squares(diagonal);

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
    namespace starting_masks = bitboard_masks::starting;

    SECTION("White")
    {
        SECTION("Pawns")
        {
            static constexpr auto pos = starting_masks::white::PAWNS;

            STATIC_REQUIRE(pos.count() == 8uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::Two);
            }
        }

        SECTION("Rooks")
        {
            static constexpr auto pos = starting_masks::white::ROOKS;

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(((square.file == File::A) or (square.file == File::H)));
            }
        }

        SECTION("Knights")
        {
            static constexpr auto pos = starting_masks::white::KNIGHTS;

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(((square.file == File::B) or (square.file == File::G)));
            }
        }

        SECTION("Bishops")
        {
            static constexpr auto pos = starting_masks::white::BISHOPS;

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(((square.file == File::C) or (square.file == File::F)));
            }
        }

        SECTION("Queen")
        {
            static constexpr auto pos = starting_masks::white::QUEEN;

            STATIC_REQUIRE(pos.count() == 1uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_white_territory());
                REQUIRE(square.rank == Rank::One);
                REQUIRE(square.file == File::D);
            }
        }

        SECTION("King")
        {
            static constexpr auto pos = starting_masks::white::KING;

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
            static constexpr auto pos = starting_masks::black::PAWNS;

            STATIC_REQUIRE(pos.count() == 8uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Seven);
            }
        }

        SECTION("Rooks")
        {
            static constexpr auto pos = starting_masks::black::ROOKS;

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(((square.file == File::A) or (square.file == File::H)));
            }
        }

        SECTION("Knights")
        {
            static constexpr auto pos = starting_masks::black::KNIGHTS;

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(((square.file == File::B) or (square.file == File::G)));
            }
        }

        SECTION("Bishops")
        {
            static constexpr auto pos = starting_masks::black::BISHOPS;

            STATIC_REQUIRE(pos.count() == 2uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(((square.file == File::C) or (square.file == File::F)));
            }
        }

        SECTION("Queen")
        {
            static constexpr auto pos = starting_masks::black::QUEEN;

            STATIC_REQUIRE(pos.count() == 1uz);

            for (const auto square : pos.squares()) {
                REQUIRE(square.is_black_territory());
                REQUIRE(square.rank == Rank::Eight);
                REQUIRE(square.file == File::D);
            }
        }

        SECTION("King")
        {
            static constexpr auto pos = starting_masks::black::KING;

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
    static constexpr auto center = bitboard_masks::CENTER;

    STATIC_REQUIRE(center.count() == 4uz);

    for (const auto [file, rank] : center.squares()) {
        REQUIRE(((file == File::D) or (file == File::E)));
        REQUIRE(((rank == Rank::Four) or (rank == Rank::Five)));
    }
}

TEST_CASE("Perimeter mask", TAGS)
{
    static constexpr auto perimeter = bitboard_masks::PERIMETER;

    STATIC_REQUIRE(perimeter.count() == 28uz);

    for (const auto rank : enum_values<Rank>()) {
        REQUIRE(perimeter.test(Square { File::A, rank }));
        REQUIRE(perimeter.test(Square { File::H, rank }));
    }

    for (const auto file : enum_values<File>()) {
        REQUIRE(perimeter.test(Square { file, Rank::One }));
        REQUIRE(perimeter.test(Square { file, Rank::Eight }));
    }
}
