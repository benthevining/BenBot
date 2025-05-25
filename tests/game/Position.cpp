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
