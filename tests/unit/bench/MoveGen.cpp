/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

static constexpr auto TAGS { "[moves][!benchmark]" };

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::pieces::Color;

TEST_CASE("Benchmarking move patterns", TAGS)
{
    namespace move_gen = chess::moves::patterns;

    // NB. we have to make sure this isn't a compile-time constant, or the
    // functions we're trying to measure will be optimized away
    chess::board::Bitboard board;

    board.set(Square { File::D, Rank::Four });
    board.set(Square { File::F, Rank::Six });

    BENCHMARK("Pawn pushes")
    {
        return move_gen::pawn_pushes<Color::White>(board);
    };

    BENCHMARK("Pawn double pushes")
    {
        return move_gen::pawn_double_pushes<Color::White>(board);
    };

    BENCHMARK("Pawn attacks")
    {
        return move_gen::pawn_attacks<Color::White>(board);
    };

    BENCHMARK("Knight")
    {
        return move_gen::knight(board);
    };

    BENCHMARK("Bishops")
    {
        return move_gen::bishop(board);
    };

    BENCHMARK("Rooks")
    {
        return move_gen::rook(board);
    };

    BENCHMARK("Queen")
    {
        return move_gen::queen(board);
    };

    BENCHMARK("King")
    {
        return move_gen::king(board);
    };
}

TEST_CASE("Benchmarking pseudo-legal move generation", TAGS)
{
    namespace move_gen = chess::moves::pseudo_legal;

    // NB. intentionally not constexpr
    chess::game::Position position {};

    const auto occupiedSquares = position.occupied();
    const auto emptySquares    = position.free();

    BENCHMARK("Pawn pushes")
    {
        return move_gen::pawn_pushes<Color::White>(
            position.whitePieces.pawns, emptySquares);
    };

    BENCHMARK("Pawn double pushes")
    {
        return move_gen::pawn_double_pushes<Color::White>(
            position.whitePieces.pawns, occupiedSquares);
    };

    BENCHMARK("Pawn captures")
    {
        return move_gen::pawn_captures<Color::White>(
            position.whitePieces.pawns, position.blackPieces.occupied);
    };

    BENCHMARK("Knights")
    {
        return move_gen::knight(
            position.whitePieces.knights, position.whitePieces.occupied);
    };

    BENCHMARK("Bishops")
    {
        return move_gen::bishop(
            position.whitePieces.bishops, emptySquares, position.whitePieces.occupied);
    };

    BENCHMARK("Rooks")
    {
        return move_gen::rook(
            position.whitePieces.rooks, emptySquares, position.whitePieces.occupied);
    };

    BENCHMARK("Queens")
    {
        return move_gen::queen(
            position.whitePieces.queens, emptySquares, position.whitePieces.occupied);
    };

    BENCHMARK("King")
    {
        return move_gen::king(
            position.whitePieces.king, position.whitePieces.occupied);
    };
}
