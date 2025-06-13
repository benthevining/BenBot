/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <charconv>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <exception>
#include <iterator>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <libchess/util/Strings.hpp>
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

    if (args.size() < 2uz) {
        std::println("Usage:");
        std::println("{} <fen> <depth>", programName);
        return EXIT_FAILURE;
    }

    const auto fenString = args.front();

    args = args.subspan(1uz);

    const auto depthString = args.front();

    const chess::search::Options options {
        .position = chess::notation::from_fen(fenString),
        .depth    = chess::util::int_from_string(depthString, 4uz)
    };

    chess::search::TranspositionTable transTable;

    const auto move = chess::search::find_best_move(options, transTable);

    std::println("{}",
        chess::notation::to_alg(options.position, move));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
