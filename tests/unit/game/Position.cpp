/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <vector>

static constexpr auto TAGS { "[game][Position]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::game::Position;
using chess::pieces::Color;

TEST_CASE("Position - starting", TAGS)
{
    static constexpr Position pos;

    STATIC_REQUIRE(pos.sideToMove == Color::White);
    STATIC_REQUIRE(! pos.enPassantTargetSquare.has_value());

    static constexpr auto occupied = pos.occupied();

    STATIC_REQUIRE(occupied.count() == 32uz);

    for (const auto square : occupied.squares()) {
        REQUIRE(((square.rank == Rank::One) || (square.rank == Rank::Two)
                 || (square.rank == Rank::Seven) || (square.rank == Rank::Eight)));
    }
}

TEST_CASE("Position - is_file_open()/get_open_files()", TAGS)
{
    static constexpr auto allFiles = magic_enum::enum_values<File>();

    Position pos;

    for (const auto file : allFiles)
        REQUIRE(! pos.is_file_open(file));

    REQUIRE(std::ranges::empty(pos.get_open_files()));

    // half open files
    pos.whitePieces.pawns.clear();

    for (const auto file : allFiles)
        REQUIRE(! pos.is_file_open(file));

    REQUIRE(std::ranges::empty(pos.get_open_files()));

    // open files
    pos.blackPieces.pawns.clear();

    for (const auto file : allFiles)
        REQUIRE(pos.is_file_open(file));

    const auto openFiles = pos.get_open_files() | std::ranges::to<std::vector>();

    REQUIRE(std::ranges::equal(openFiles, allFiles));
}

TEST_CASE("Position - is_file_half_open()/get_half_open_files()", TAGS)
{
    static constexpr auto allFiles = magic_enum::enum_values<File>();

    Position pos;

    for (const auto file : allFiles)
        REQUIRE(! pos.is_file_half_open(file));

    REQUIRE(std::ranges::empty(pos.get_half_open_files()));

    // half open files
    pos.whitePieces.pawns.clear();

    for (const auto file : allFiles)
        REQUIRE(pos.is_file_half_open(file));

    const auto files = pos.get_half_open_files() | std::ranges::to<std::vector>();

    REQUIRE(std::ranges::equal(files, allFiles));

    // open files
    pos.blackPieces.pawns.clear();

    for (const auto file : allFiles)
        REQUIRE(! pos.is_file_half_open(file));

    REQUIRE(std::ranges::empty(pos.get_half_open_files()));
}

TEST_CASE("Position - is_check()", TAGS)
{
    using chess::notation::from_fen;

    STATIC_REQUIRE(! Position {}.is_check());

    REQUIRE(! from_fen("r1bqkb1r/pppppppp/2n4n/8/2B1P3/5Q2/PPPP1PPP/RNB1K1NR b KQkq - 4 3")
            .is_check());

    {
        const auto check = from_fen("r1bqkb1r/pppppB1p/2n4n/6p1/4P3/5Q2/PPPP1PPP/RNB1K1NR b KQkq - 0 4");

        REQUIRE(check.is_check());
        REQUIRE(! check.is_checkmate());
    }

    {
        const auto mate = from_fen("1rbqkbnr/p1pppQpp/1pn5/8/2B1P3/8/PPPP1PPP/RNB1K1NR b KQk - 0 4");

        REQUIRE(mate.is_check());

        REQUIRE(mate.is_checkmate());
    }
}
