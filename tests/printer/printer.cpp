/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/notation/FEN.hpp>
#include <print>
#include <string>

int main()
{
    using chess::game::print_utf8;

    static const std::string fen {
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    };

    std::println("{}",
        print_utf8(chess::notation::from_fen(fen)));

    std::println("{}", fen);
}
