/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/Masks.hpp>

static constexpr auto TAGS { "[board][Bitboard][!benchmark]" };

TEST_CASE("Benchmarking bitboard iteration", TAGS)
{
    static constexpr auto board = chess::board::masks::starting::black::PAWNS;

    BENCHMARK("Iterate bitboard indices")
    {
        chess::board::BitboardIndex total { 0 };

        for (const auto idx : board.indices())
            total += idx; // cppcheck-suppress useStlAlgorithm

        return total;
    };
}
