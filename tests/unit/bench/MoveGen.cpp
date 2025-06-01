/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

static constexpr auto TAGS { "[moves][!benchmark]" };

using chess::pieces::Color;

namespace starting_masks = chess::board::masks::starting;

static constexpr auto STARTING_PAWNS   = starting_masks::white::PAWNS;
static constexpr auto STARTING_KNIGHTS = starting_masks::white::KNIGHTS;
static constexpr auto STARTING_BISHOPS = starting_masks::white::BISHOPS;
static constexpr auto STARTING_ROOKS   = starting_masks::white::ROOKS;
static constexpr auto STARTING_QUEEN   = starting_masks::white::QUEEN;
static constexpr auto STARTING_KING    = starting_masks::white::KING;

TEST_CASE("Benchmarking move patterns", TAGS)
{
    namespace move_gen = chess::moves::patterns;

    BENCHMARK("Pawn pushes")
    {
        return move_gen::pawn_pushes<Color::White>(STARTING_PAWNS);
    };

    BENCHMARK("Pawn double pushes")
    {
        return move_gen::pawn_double_pushes<Color::White>(STARTING_PAWNS);
    };

    BENCHMARK("Pawn attacks")
    {
        return move_gen::pawn_attacks<Color::White>(STARTING_PAWNS);
    };

    BENCHMARK("Knight")
    {
        return move_gen::knight(STARTING_KNIGHTS);
    };

    BENCHMARK("Bishops")
    {
        return move_gen::bishop(STARTING_BISHOPS);
    };

    BENCHMARK("Rooks")
    {
        return move_gen::rook(STARTING_ROOKS);
    };

    BENCHMARK("Queen")
    {
        return move_gen::queen(STARTING_QUEEN);
    };

    BENCHMARK("King")
    {
        return move_gen::king(STARTING_KING);
    };
}

TEST_CASE("Benchmarking pseudo-legal move generation", TAGS)
{
    namespace move_gen = chess::moves::pseudo_legal;

    static constexpr chess::game::Position position {};

    static constexpr auto occupiedSquares = position.occupied();
    static constexpr auto emptySquares    = position.free();

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
