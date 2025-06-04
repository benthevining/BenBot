/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

// this executable is invoked with a single positional argument,
// the FEN string of the position to solve, and prints the best
// move in algebraic notation to stdout.

#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <exception>
#include <iterator>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <print>
#include <span>
#include <string_view>
#include <vector>

int main(const int argc, const char** argv)
try {
    const std::vector<std::string_view> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    const auto programName = args.front();

    args = args.subspan(1uz);

    if (args.empty()) {
        std::println("Usage:");
        std::println("{} <fen>", programName);
        return EXIT_FAILURE;
    }

    const auto fenString = args.front();

    const auto position = chess::notation::from_fen(fenString);

    const auto move = chess::search::find_best_move(position);

    std::println("{}",
        chess::notation::to_alg(position, move));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
