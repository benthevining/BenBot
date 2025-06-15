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
#include <print>
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
    const Position pos {};

    REQUIRE(pos.sideToMove == Color::White);
    REQUIRE(! pos.enPassantTargetSquare.has_value());

    const auto occupied = pos.occupied();

    REQUIRE(occupied.count() == 32uz);

    for (const auto [file, rank] : occupied.squares()) {
        REQUIRE(((rank == Rank::One) || (rank == Rank::Two)
                 || (rank == Rank::Seven) || (rank == Rank::Eight)));
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

    SECTION("Starting position")
    {
        const Position startingPosition {};

        REQUIRE(! startingPosition.is_check());
        REQUIRE(! startingPosition.is_checkmate());
        REQUIRE(! startingPosition.is_stalemate());
    }

    SECTION("Blocked ray attack")
    {
        const auto pos = from_fen("r1bqkb1r/pppppppp/2n4n/8/2B1P3/5Q2/PPPP1PPP/RNB1K1NR b KQkq - 4 3");

        REQUIRE(! pos.is_check());
        REQUIRE(! pos.is_checkmate());
        REQUIRE(! pos.is_stalemate());
    }

    SECTION("Check")
    {
        const auto pos = from_fen("r1bqkb1r/pppppB1p/2n4n/6p1/4P3/5Q2/PPPP1PPP/RNB1K1NR b KQkq - 0 4");

        REQUIRE(pos.is_check());
        REQUIRE(! pos.is_checkmate());
        REQUIRE(! pos.is_stalemate());
    }

    SECTION("Checkmate")
    {
        const auto pos = from_fen("1rbqkbnr/p1pppQpp/1pn5/8/2B1P3/8/PPPP1PPP/RNB1K1NR b KQk - 0 4");

        REQUIRE(pos.is_check());
        REQUIRE(pos.is_checkmate());
        REQUIRE(! pos.is_stalemate());
    }

    SECTION("Stalemate")
    {
        const auto pos = from_fen("7K/5k2/6q1/8/8/8/8/8 w - - 0 1");

        REQUIRE(! pos.is_check());
        REQUIRE(! pos.is_checkmate());
        REQUIRE(pos.is_stalemate());
    }
}

TEST_CASE("Position - passed pawns", TAGS)
{
    using chess::notation::from_fen;

    SECTION("Starting position")
    {
        const Position startingPosition {};

        REQUIRE(startingPosition.get_passed_pawns<Color::White>().none());
        REQUIRE(startingPosition.get_passed_pawns<Color::Black>().none());
    }

    SECTION("White and Black each have a passer")
    {
        const auto position = from_fen("8/8/2Pk4/8/8/5p2/5K2/8 w - - 0 1");

        REQUIRE(position.get_passed_pawns<Color::White>().count() == 1uz);
        REQUIRE(position.get_passed_pawns<Color::Black>().count() == 1uz);
    }

    SECTION("White has a passer")
    {
        const auto position = from_fen("8/3Pp3/2p1P3/2P5/1k1K4/5p2/5P2/8 w - - 0 1");

        REQUIRE(position.get_passed_pawns<Color::White>().count() == 1uz);
        REQUIRE(position.get_passed_pawns<Color::Black>().count() == 0uz);
    }
}
