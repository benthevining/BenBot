/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>

static constexpr auto TAGS { "[board][Pieces]" };

using chess::board::File;
using chess::board::Pieces;
using chess::board::Rank;
using chess::board::Square;
using chess::pieces::Color;

TEST_CASE("Pieces - starting position", TAGS)
{
    SECTION("White")
    {
        static constexpr Pieces pieces { Color::White };

        STATIC_REQUIRE(pieces.material() == 41uz);

        static constexpr auto occupied = pieces.occupied;

        STATIC_REQUIRE(occupied.count() == 16uz);

        for (const auto [file, rank] : occupied.squares())
            REQUIRE(((rank == Rank::One) || (rank == Rank::Two)));
    }

    SECTION("Black")
    {
        static constexpr Pieces pieces { Color::Black };

        STATIC_REQUIRE(pieces.material() == 41uz);

        static constexpr auto occupied = pieces.occupied;

        STATIC_REQUIRE(occupied.count() == 16uz);

        for (const auto [file, rank] : occupied.squares())
            REQUIRE(((rank == Rank::Seven) || (rank == Rank::Eight)));
    }
}

TEST_CASE("Pieces - is_file_half_open()", TAGS)
{
    Pieces pieces { Color::White };

    for (const auto file : magic_enum::enum_values<File>())
        REQUIRE(! pieces.is_file_half_open(file));

    pieces.pawns.unset(Square { File::A, Rank::Two });

    REQUIRE(pieces.is_file_half_open(File::A));
}

TEST_CASE("Pieces - has_bishop_pair()", TAGS)
{
    Pieces pieces { Color::White };

    REQUIRE(pieces.has_bishop_pair());

    // remove LSB
    pieces.bishops.unset(Square { File::F, Rank::One });

    REQUIRE(! pieces.has_bishop_pair());

    // add another DSB
    pieces.bishops.set(Square { File::A, Rank::Five });

    REQUIRE(! pieces.has_bishop_pair());

    // add LSB
    pieces.bishops.set(Square { File::E, Rank::Four });

    REQUIRE(pieces.has_bishop_pair());

    // remove all
    pieces.bishops.clear();

    REQUIRE(! pieces.has_bishop_pair());
}

TEST_CASE("Pieces - get_piece_on()", TAGS)
{
    static constexpr Pieces pieces { Color::White };

    STATIC_REQUIRE(! pieces.get_piece_on(Square { File::A, Rank::Three }).has_value());
    STATIC_REQUIRE(! pieces.get_piece_on(Square { File::C, Rank::Four }).has_value());
    STATIC_REQUIRE(! pieces.get_piece_on(Square { File::E, Rank::Eight }).has_value());
    STATIC_REQUIRE(! pieces.get_piece_on(Square { File::H, Rank::Five }).has_value());

    using PieceType = chess::pieces::Type;

    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::A, Rank::One }) == PieceType::Rook);
    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::H, Rank::One }) == PieceType::Rook);

    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::B, Rank::One }) == PieceType::Knight);
    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::G, Rank::One }) == PieceType::Knight);

    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::C, Rank::One }) == PieceType::Bishop);
    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::F, Rank::One }) == PieceType::Bishop);

    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::D, Rank::One }) == PieceType::Queen);

    STATIC_REQUIRE(*pieces.get_piece_on(Square { File::E, Rank::One }) == PieceType::King);

    for (auto file : magic_enum::enum_values<File>())
        REQUIRE(*pieces.get_piece_on(Square { file, Rank::Two }) == PieceType::Pawn);
}
