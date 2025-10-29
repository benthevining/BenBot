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
#include <catch2/generators/catch_generators_range.hpp>
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <magic_enum/magic_enum.hpp>
#include <string_view>

inline constexpr auto TAGS { "[board][Square]" };

using chess::board::BitboardIndex;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using std::string_view;

namespace gen = Catch::Generators;

namespace {
void square_to_from_idx(
    const string_view sectionName,
    const File file, const Rank rank,
    const size_t index)
{
    SECTION(sectionName.data())
    {
        const Square square { file, rank };
        REQUIRE(square.index() == index);
        REQUIRE(square == Square::from_index(static_cast<BitboardIndex>(index)));
    }
}
} // namespace

TEST_CASE("Square - to/from index", TAGS)
{
    square_to_from_idx("A1", File::A, Rank::One, 0uz);
    square_to_from_idx("B1", File::B, Rank::One, 1uz);
    square_to_from_idx("C1", File::C, Rank::One, 2uz);
    square_to_from_idx("D1", File::D, Rank::One, 3uz);
    square_to_from_idx("E1", File::E, Rank::One, 4uz);
    square_to_from_idx("F1", File::F, Rank::One, 5uz);
    square_to_from_idx("G1", File::G, Rank::One, 6uz);
    square_to_from_idx("H1", File::H, Rank::One, 7uz);

    square_to_from_idx("A2", File::A, Rank::Two, 8uz);
    square_to_from_idx("B2", File::B, Rank::Two, 9uz);
    square_to_from_idx("C2", File::C, Rank::Two, 10uz);
    square_to_from_idx("D2", File::D, Rank::Two, 11uz);
    square_to_from_idx("E2", File::E, Rank::Two, 12uz);
    square_to_from_idx("F2", File::F, Rank::Two, 13uz);
    square_to_from_idx("G2", File::G, Rank::Two, 14uz);
    square_to_from_idx("H2", File::H, Rank::Two, 15uz);

    square_to_from_idx("A3", File::A, Rank::Three, 16uz);
    square_to_from_idx("B3", File::B, Rank::Three, 17uz);
    square_to_from_idx("C3", File::C, Rank::Three, 18uz);
    square_to_from_idx("D3", File::D, Rank::Three, 19uz);
    square_to_from_idx("E3", File::E, Rank::Three, 20uz);
    square_to_from_idx("F3", File::F, Rank::Three, 21uz);
    square_to_from_idx("G3", File::G, Rank::Three, 22uz);
    square_to_from_idx("H3", File::H, Rank::Three, 23uz);

    square_to_from_idx("A4", File::A, Rank::Four, 24uz);
    square_to_from_idx("B4", File::B, Rank::Four, 25uz);
    square_to_from_idx("C4", File::C, Rank::Four, 26uz);
    square_to_from_idx("D4", File::D, Rank::Four, 27uz);
    square_to_from_idx("E4", File::E, Rank::Four, 28uz);
    square_to_from_idx("F4", File::F, Rank::Four, 29uz);
    square_to_from_idx("G4", File::G, Rank::Four, 30uz);
    square_to_from_idx("H4", File::H, Rank::Four, 31uz);

    square_to_from_idx("A5", File::A, Rank::Five, 32uz);
    square_to_from_idx("B5", File::B, Rank::Five, 33uz);
    square_to_from_idx("C5", File::C, Rank::Five, 34uz);
    square_to_from_idx("D5", File::D, Rank::Five, 35uz);
    square_to_from_idx("E5", File::E, Rank::Five, 36uz);
    square_to_from_idx("F5", File::F, Rank::Five, 37uz);
    square_to_from_idx("G5", File::G, Rank::Five, 38uz);
    square_to_from_idx("H5", File::H, Rank::Five, 39uz);

    square_to_from_idx("A6", File::A, Rank::Six, 40uz);
    square_to_from_idx("B6", File::B, Rank::Six, 41uz);
    square_to_from_idx("C6", File::C, Rank::Six, 42uz);
    square_to_from_idx("D6", File::D, Rank::Six, 43uz);
    square_to_from_idx("E6", File::E, Rank::Six, 44uz);
    square_to_from_idx("F6", File::F, Rank::Six, 45uz);
    square_to_from_idx("G6", File::G, Rank::Six, 46uz);
    square_to_from_idx("H6", File::H, Rank::Six, 47uz);

    square_to_from_idx("A7", File::A, Rank::Seven, 48uz);
    square_to_from_idx("B7", File::B, Rank::Seven, 49uz);
    square_to_from_idx("C7", File::C, Rank::Seven, 50uz);
    square_to_from_idx("D7", File::D, Rank::Seven, 51uz);
    square_to_from_idx("E7", File::E, Rank::Seven, 52uz);
    square_to_from_idx("F7", File::F, Rank::Seven, 53uz);
    square_to_from_idx("G7", File::G, Rank::Seven, 54uz);
    square_to_from_idx("H7", File::H, Rank::Seven, 55uz);

    square_to_from_idx("A8", File::A, Rank::Eight, 56uz);
    square_to_from_idx("B8", File::B, Rank::Eight, 57uz);
    square_to_from_idx("C8", File::C, Rank::Eight, 58uz);
    square_to_from_idx("D8", File::D, Rank::Eight, 59uz);
    square_to_from_idx("E8", File::E, Rank::Eight, 60uz);
    square_to_from_idx("F8", File::F, Rank::Eight, 61uz);
    square_to_from_idx("G8", File::G, Rank::Eight, 62uz);
    square_to_from_idx("H8", File::H, Rank::Eight, 63uz);
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

namespace {
void square_to_from_string(
    const string_view str, const string_view strLower,
    const File file, const Rank rank)
{
    const Square square { file, rank };
    REQUIRE(square == Square::from_string(str));
    REQUIRE(square == Square::from_string(strLower));
    REQUIRE(std::format("{}", square) == strLower);
}
} // namespace

TEST_CASE("Square - to/from string", TAGS)
{
    square_to_from_string("A1", "a1", File::A, Rank::One);
    square_to_from_string("B1", "b1", File::B, Rank::One);
    square_to_from_string("C1", "c1", File::C, Rank::One);
    square_to_from_string("D1", "d1", File::D, Rank::One);
    square_to_from_string("E1", "e1", File::E, Rank::One);
    square_to_from_string("F1", "f1", File::F, Rank::One);
    square_to_from_string("G1", "g1", File::G, Rank::One);
    square_to_from_string("H1", "h1", File::H, Rank::One);

    square_to_from_string("A2", "a2", File::A, Rank::Two);
    square_to_from_string("B2", "b2", File::B, Rank::Two);
    square_to_from_string("C2", "c2", File::C, Rank::Two);
    square_to_from_string("D2", "d2", File::D, Rank::Two);
    square_to_from_string("E2", "e2", File::E, Rank::Two);
    square_to_from_string("F2", "f2", File::F, Rank::Two);
    square_to_from_string("G2", "g2", File::G, Rank::Two);
    square_to_from_string("H2", "h2", File::H, Rank::Two);

    square_to_from_string("A3", "a3", File::A, Rank::Three);
    square_to_from_string("B3", "b3", File::B, Rank::Three);
    square_to_from_string("C3", "c3", File::C, Rank::Three);
    square_to_from_string("D3", "d3", File::D, Rank::Three);
    square_to_from_string("E3", "e3", File::E, Rank::Three);
    square_to_from_string("F3", "f3", File::F, Rank::Three);
    square_to_from_string("G3", "g3", File::G, Rank::Three);
    square_to_from_string("H3", "h3", File::H, Rank::Three);

    square_to_from_string("A4", "a4", File::A, Rank::Four);
    square_to_from_string("B4", "b4", File::B, Rank::Four);
    square_to_from_string("C4", "c4", File::C, Rank::Four);
    square_to_from_string("D4", "d4", File::D, Rank::Four);
    square_to_from_string("E4", "e4", File::E, Rank::Four);
    square_to_from_string("F4", "f4", File::F, Rank::Four);
    square_to_from_string("G4", "g4", File::G, Rank::Four);
    square_to_from_string("H4", "h4", File::H, Rank::Four);

    square_to_from_string("A5", "a5", File::A, Rank::Five);
    square_to_from_string("B5", "b5", File::B, Rank::Five);
    square_to_from_string("C5", "c5", File::C, Rank::Five);
    square_to_from_string("D5", "d5", File::D, Rank::Five);
    square_to_from_string("E5", "e5", File::E, Rank::Five);
    square_to_from_string("F5", "f5", File::F, Rank::Five);
    square_to_from_string("G5", "g5", File::G, Rank::Five);
    square_to_from_string("H5", "h5", File::H, Rank::Five);

    square_to_from_string("A6", "a6", File::A, Rank::Six);
    square_to_from_string("B6", "b6", File::B, Rank::Six);
    square_to_from_string("C6", "c6", File::C, Rank::Six);
    square_to_from_string("D6", "d6", File::D, Rank::Six);
    square_to_from_string("E6", "e6", File::E, Rank::Six);
    square_to_from_string("F6", "f6", File::F, Rank::Six);
    square_to_from_string("G6", "g6", File::G, Rank::Six);
    square_to_from_string("H6", "h6", File::H, Rank::Six);

    square_to_from_string("A7", "a7", File::A, Rank::Seven);
    square_to_from_string("B7", "b7", File::B, Rank::Seven);
    square_to_from_string("C7", "c7", File::C, Rank::Seven);
    square_to_from_string("D7", "d7", File::D, Rank::Seven);
    square_to_from_string("E7", "e7", File::E, Rank::Seven);
    square_to_from_string("F7", "f7", File::F, Rank::Seven);
    square_to_from_string("G7", "g7", File::G, Rank::Seven);
    square_to_from_string("H7", "h7", File::H, Rank::Seven);

    square_to_from_string("A8", "a8", File::A, Rank::Eight);
    square_to_from_string("B8", "b8", File::B, Rank::Eight);
    square_to_from_string("C8", "c8", File::C, Rank::Eight);
    square_to_from_string("D8", "d8", File::D, Rank::Eight);
    square_to_from_string("E8", "e8", File::E, Rank::Eight);
    square_to_from_string("F8", "f8", File::F, Rank::Eight);
    square_to_from_string("G8", "g8", File::G, Rank::Eight);
    square_to_from_string("H8", "h8", File::H, Rank::Eight);
}

TEST_CASE("Square - is_white_territory()/is_black_territory()", TAGS)
{
    SECTION("is_white_territory()")
    {
        const auto index = GENERATE(gen::range(0uz, 32uz));

        const auto square = Square::from_index(static_cast<BitboardIndex>(index));

        REQUIRE(square.is_white_territory());
    }

    SECTION("is_black_territory()")
    {
        const auto index = GENERATE(gen::range(32uz, 64uz));

        const auto square = Square::from_index(static_cast<BitboardIndex>(index));

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
            REQUIRE(not square.is_kingside());
        }
    }

    SECTION("Kingside")
    {
        const auto file = static_cast<File>(
            GENERATE(gen::range(4uz, 8uz)));

        for (const auto rank : magic_enum::enum_values<Rank>()) {
            const Square square { file, rank };

            REQUIRE(square.is_kingside());
            REQUIRE(not square.is_queenside());
        }
    }
}
