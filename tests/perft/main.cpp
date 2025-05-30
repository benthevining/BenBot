/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdlib>
#include <exception>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Perft.hpp>
#include <libchess/notation/FEN.hpp>
#include <print>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

// TODO:
// print number of nodes from each starting move
// option to output JSON?

struct PerftOptions final {
    chess::game::Position startingPosition {};

    std::size_t depth { 1uz };
};

namespace {

void print_help(const std::string_view programName)
{
    std::println("Usage:");
    std::println("{} <depth> [--fen \"<fenString>\"]", programName);
}

[[nodiscard]] PerftOptions parse_options(std::span<const std::string_view> args)
{
    PerftOptions options;

    while (! args.empty()) {
        const auto arg = args.front();

        args = args.subspan(1uz);

        if (arg == "--fen") {
            if (args.empty())
                throw std::invalid_argument {
                    "Error: expected FEN string following option --fen"
                };

            const auto fen = args.front();

            args = args.subspan(1uz);

            options.startingPosition = chess::notation::from_fen(fen);

            continue;
        }

        // assume arg is depth
        std::from_chars(arg.begin(), arg.end(), options.depth);
    };

    return options;
}

void run_perft(const PerftOptions& options)
{
    using Clock = std::chrono::high_resolution_clock;

    std::println("Starting position:");
    std::println("{}", chess::game::print_utf8(options.startingPosition));

    std::println("Running perft depth {}...", options.depth);
    std::println("");

    const auto startTime = Clock::now();

    const auto result = chess::moves::perft(options.depth, options.startingPosition);

    const auto endTime = Clock::now();

    std::println("Nodes: {}", result.nodes);
    std::println("Captures: {}", result.captures);
    std::println("En passant captures: {}", result.enPassantCaptures);
    std::println("Castles: {}", result.castles);
    std::println("Promotions: {}", result.promotions);
    std::println("Checks: {}", result.checks);
    std::println("Checkmates: {}", result.checkmates);
    std::println("Stalemates: {}", result.stalemates);

    const auto wallTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);

    std::println("");
    std::println("Search time: {}", wallTime);
}

} // namespace

int main(const int argc, const char** argv)
try {
    const std::vector<std::string_view> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    const auto programName = args.front();

    args = args.subspan(1uz);

    if (args.empty()
        || std::ranges::contains(args, "--help")) {
        print_help(programName);
        return EXIT_FAILURE;
    }

    run_perft(parse_options(args));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
