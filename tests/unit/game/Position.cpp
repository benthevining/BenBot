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

using chess::notation::from_fen;

TEST_CASE("Position - starting", TAGS)
{
    const Position pos {};

    REQUIRE(pos.sideToMove == Color::White);
    REQUIRE(not pos.enPassantTargetSquare.has_value());

    const auto occupied = pos.occupied();

    REQUIRE(occupied.count() == 32uz);

    for (const auto [file, rank] : occupied.squares()) {
        REQUIRE(((rank == Rank::One) or (rank == Rank::Two)
                 or (rank == Rank::Seven) or (rank == Rank::Eight)));
    }
}

TEST_CASE("Position - is_file_open()/get_open_files()", TAGS)
{
    static constexpr auto allFiles = magic_enum::enum_values<File>();

    Position pos;

    for (const auto file : allFiles)
        REQUIRE(not pos.is_file_open(file));

    REQUIRE(std::ranges::empty(pos.get_open_files()));

    // half open files
    pos.whitePieces.pawns.clear();

    for (const auto file : allFiles)
        REQUIRE(not pos.is_file_open(file));

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
        REQUIRE(not pos.is_file_half_open(file));

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
        REQUIRE(not pos.is_file_half_open(file));

    REQUIRE(std::ranges::empty(pos.get_half_open_files()));
}

TEST_CASE("Position - is_check()", TAGS)
{
    SECTION("Starting position")
    {
        const Position startingPosition {};

        REQUIRE(not startingPosition.is_check());
        REQUIRE(not startingPosition.is_checkmate());
        REQUIRE(not startingPosition.is_stalemate());
    }

    SECTION("Blocked ray attack")
    {
        const auto pos = from_fen("r1bqkb1r/pppppppp/2n4n/8/2B1P3/5Q2/PPPP1PPP/RNB1K1NR b KQkq - 4 3");

        REQUIRE(not pos.is_check());
        REQUIRE(not pos.is_checkmate());
        REQUIRE(not pos.is_stalemate());
    }

    SECTION("Check")
    {
        const auto pos = from_fen("r1bqkb1r/pppppB1p/2n4n/6p1/4P3/5Q2/PPPP1PPP/RNB1K1NR b KQkq - 0 4");

        REQUIRE(pos.is_check());
        REQUIRE(not pos.is_checkmate());
        REQUIRE(not pos.is_stalemate());
    }

    SECTION("Checkmate")
    {
        const auto pos = from_fen("1rbqkbnr/p1pppQpp/1pn5/8/2B1P3/8/PPPP1PPP/RNB1K1NR b KQk - 0 4");

        REQUIRE(pos.is_check());
        REQUIRE(pos.is_checkmate());
        REQUIRE(not pos.is_stalemate());
    }

    SECTION("Stalemate")
    {
        const auto pos = from_fen("7K/5k2/6q1/8/8/8/8/8 w - - 0 1");

        REQUIRE(not pos.is_check());
        REQUIRE(not pos.is_checkmate());
        REQUIRE(pos.is_stalemate());
    }
}

TEST_CASE("Position - stalemate", TAGS)
{
    SECTION("White is stalemated")
    {
        const auto position = from_fen("7K/5k2/6q1/8/8/8/8/8 w - - 0 1");

        REQUIRE(position.is_stalemate());
    }

    SECTION("Black is stalemated")
    {
        const auto position = from_fen("2k5/P7/2K5/6B1/8/8/8/8 b - - 0 1");

        REQUIRE(position.is_stalemate());
    }
}

TEST_CASE("Position - checkmate", TAGS)
{
    SECTION("White is checkmated")
    {
        const auto position = from_fen("rnb1kb1r/pppppppp/4q3/8/2P5/1B1n4/PP1PPPPP/RN1QKBNR w KQkq - 0 1");

        REQUIRE(position.is_checkmate());
    }

    SECTION("Black is checkmated")
    {
        const auto position = from_fen("r2qkbnr/ppp1pBpp/2n5/1b1pN3/8/4PQ2/PPPP1PPP/R1B1K1NR b KQkq - 0 1");

        REQUIRE(position.is_checkmate());
    }
}

TEST_CASE("Position - draw by insufficient material", TAGS)
{
    SECTION("Lone kings")
    {
        const auto position = from_fen("8/8/1K6/8/5k2/8/8/8 w - - 0 1");

        REQUIRE(position.is_draw_by_insufficient_material());
    }

    SECTION("White has a single knight")
    {
        const auto position = from_fen("8/8/1K6/8/5k2/3N4/8/8 b - - 0 1");

        REQUIRE(position.is_draw_by_insufficient_material());
    }

    SECTION("White has a single bishop")
    {
        const auto position = from_fen("8/8/1K6/8/5k2/8/8/5B2 b - - 0 1");

        REQUIRE(position.is_draw_by_insufficient_material());
    }

    SECTION("Black has a single knight")
    {
        const auto position = from_fen("8/8/1K1n4/8/5k2/8/8/8 w - - 0 1");

        REQUIRE(position.is_draw_by_insufficient_material());
    }

    SECTION("Black has a single bishop")
    {
        const auto position = from_fen("8/8/1K6/8/3b1k2/8/8/8 w - - 0 1");

        REQUIRE(position.is_draw_by_insufficient_material());
    }
}

TEST_CASE("Position - passed pawns", TAGS)
{
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

TEST_CASE("Position - backward pawns", TAGS)
{
    SECTION("Starting position")
    {
        const Position startingPosition {};

        REQUIRE(startingPosition.get_backward_pawns<Color::White>().none());
        REQUIRE(startingPosition.get_backward_pawns<Color::Black>().none());
    }

    SECTION("Telestop weakness")
    {
        const auto position = from_fen("8/5p2/6p1/p1p3P1/P1P5/7P/1P6/8 w - - 0 1");

        const auto wBackwards = position.get_backward_pawns<Color::White>();

        REQUIRE(wBackwards.count() == 2uz);

        REQUIRE(wBackwards.test(Square { .file = File::B, .rank = Rank::Two }));
        REQUIRE(wBackwards.test(Square { .file = File::H, .rank = Rank::Three }));

        const auto bBackwards = position.get_backward_pawns<Color::Black>();

        REQUIRE(bBackwards.count() == 3uz);

        REQUIRE(bBackwards.test(Square { .file = File::A, .rank = Rank::Five }));
        REQUIRE(bBackwards.test(Square { .file = File::C, .rank = Rank::Five }));
        REQUIRE(bBackwards.test(Square { .file = File::F, .rank = Rank::Seven }));
    }
}
