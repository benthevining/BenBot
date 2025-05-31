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
#include <filesystem>
#include <fstream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Perft.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

struct PerftOptions final {
    chess::game::Position startingPosition {};

    std::size_t depth { 1uz };

    std::optional<std::filesystem::path> jsonOutputPath;
};

using chess::moves::PerftResult;

namespace {

void print_help(const std::string_view programName)
{
    std::println("Usage:");
    std::println("{} <depth> [--fen \"<fenString>\"] [--write-json <path>]", programName);
    std::println("If the --write-json option is given, a JSON file with results will be written to the given path.");
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

        if (arg == "--write-json") {
            if (args.empty())
                throw std::invalid_argument {
                    "Error: expected path following option --write-json"
                };

            const auto path = args.front();

            args = args.subspan(1uz);

            options.jsonOutputPath = path;

            continue;
        }

        // assume arg is depth
        std::from_chars(arg.data(), arg.data() + arg.length(), options.depth);
    };

    return options;
}

void write_json_file(
    const PerftOptions& options,
    const PerftResult&  result,
    const auto          wallTime)
{
    if (! options.jsonOutputPath.has_value())
        return;

    const auto& path = *options.jsonOutputPath;

    nlohmann::json json;

    json["starting_fen"]        = chess::notation::to_fen(options.startingPosition);
    json["depth"]               = options.depth;
    json["search_time_seconds"] = wallTime.count();

    auto& result_json = json["results"];

    result_json["totalNodes"]  = result.nodes;
    result_json["captures"]    = result.captures;
    result_json["en_passants"] = result.enPassantCaptures;
    result_json["castles"]     = result.castles;
    result_json["promotions"]  = result.promotions;
    result_json["checks"]      = result.checks;
    result_json["checkmates"]  = result.checkmates;
    result_json["stalemates"]  = result.stalemates;

    std::filesystem::create_directories(path.parent_path());

    std::ofstream output { path };

    output << json.dump(1);

    std::println("Wrote JSON results to {}", path.string()); // NOLINT(build/include_what_you_use)
    std::println("");
}

void print_root_nodes(const PerftResult& result)
{
    for (const auto [move, numChildren] : result.rootNodes) {
        std::println("{} {}",
            chess::notation::to_uci(move), numChildren);
    }
}

void print_results(
    const PerftResult& result,
    const auto         wallTime)
{
    std::println("Nodes: {}", result.nodes);
    std::println("Captures: {}", result.captures);
    std::println("En passant captures: {}", result.enPassantCaptures);
    std::println("Castles: {}", result.castles);
    std::println("Promotions: {}", result.promotions);
    std::println("Checks: {}", result.checks);
    std::println("Checkmates: {}", result.checkmates);
    std::println("Stalemates: {}", result.stalemates);

    std::println("");
    std::println("Search time: {}", wallTime);
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

    const auto wallTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);

    write_json_file(options, result, wallTime);

    print_root_nodes(result);

    std::println("");

    print_results(result, wallTime);
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
