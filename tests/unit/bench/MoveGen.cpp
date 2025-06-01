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
#include <libchess/moves/Patterns.hpp>

static constexpr auto TAGS { "[moves][!benchmark]" };

namespace starting_masks = chess::board::masks::starting;

// pawn pushes, double pushes, attacks

TEST_CASE("Benchmarking move patterns", TAGS)
{
    namespace move_gen = chess::moves::patterns;

    BENCHMARK("Knight")
    {
        return move_gen::knight(starting_masks::white::KNIGHTS);
    };

    BENCHMARK("Bishops")
    {
        return move_gen::bishop(starting_masks::white::BISHOPS);
    };

    BENCHMARK("Rooks")
    {
        return move_gen::rook(starting_masks::white::ROOKS);
    };

    BENCHMARK("Queen")
    {
        return move_gen::queen(starting_masks::white::QUEEN);
    };

    BENCHMARK("King")
    {
        return move_gen::king(starting_masks::white::KING);
    };
}
