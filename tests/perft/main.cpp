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
#include <beman/inplace_vector/inplace_vector.hpp>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/Perft.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/util/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace chess {

using std::chrono::seconds;
using std::println;
using std::size_t;

struct PerftOptions final {
    game::Position startingPosition;

    size_t depth { 1uz };

    std::optional<std::filesystem::path> jsonOutputPath;
};

namespace {

    constexpr auto MAX_ARGS = 4uz;

    void print_help(const std::string_view programName)
    {
        println("Usage:");
        println("{} <depth> [--fen \"<fenString>\"] [--write-json <path>]", programName);
        println("If the --write-json option is given, a JSON file with results will be written to the given path.");
    }

    [[nodiscard]] PerftOptions parse_options(std::span<const std::string_view> args)
    {
        PerftOptions options {};

        while (not args.empty()) {
            const auto arg = args.front();

            args = args.subspan(1uz);

            if (arg == "--fen") {
                if (args.empty())
                    throw std::invalid_argument {
                        "Error: expected FEN string following option --fen"
                    };

                const auto fen = args.front();

                args = args.subspan(1uz);

                options.startingPosition = notation::from_fen(fen);

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
            options.depth = util::int_from_string(arg, options.depth);
        };

        return options;
    }

    void write_json_file(
        const PerftOptions&       options,
        const moves::PerftResult& result,
        const seconds             wallTime)
    {
        if (not options.jsonOutputPath.has_value())
            return;

        nlohmann::json json;

        json["starting_fen"]        = notation::to_fen(options.startingPosition);
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

        const auto& path = *options.jsonOutputPath;

        create_directories(path.parent_path());

        std::ofstream output { path };

        output << json.dump(1);

        println("Wrote JSON results to {}", path.string()); // NOLINT(build/include_what_you_use)
        println("");
    }

    void print_root_nodes(const moves::PerftResult& result)
    {
        for (const auto [move, numChildren] : result.rootNodes) {
            println("{} {}",
                notation::to_uci(move), numChildren);
        }
    }

    void print_results(
        const moves::PerftResult& result,
        const seconds             wallTime)
    {
        println("Nodes: {}", result.nodes);
        println("Captures: {}", result.captures);
        println("En passant captures: {}", result.enPassantCaptures);
        println("Castles: {}", result.castles);
        println("Promotions: {}", result.promotions);
        println("Checks: {}", result.checks);
        println("Checkmates: {}", result.checkmates);
        println("Stalemates: {}", result.stalemates);

        println("");
        println("Search time: {}", wallTime);
    }

    void run_perft(const PerftOptions& options)
    {
        using Clock = std::chrono::high_resolution_clock;

        println("Starting position:");
        println("{}", game::print_utf8(options.startingPosition));
        println("Running perft depth {}...", options.depth);
        println("");

        const auto startTime = Clock::now();

        const auto result = moves::perft(options.depth, options.startingPosition);

        const auto endTime = Clock::now();

        const auto wallTime = std::chrono::duration_cast<seconds>(endTime - startTime);

        write_json_file(options, result, wallTime);

        print_root_nodes(result);

        println("");

        print_results(result, wallTime);
    }

} // namespace

} // namespace chess

int main(const int argc, const char** argv)
try {
    const beman::inplace_vector<std::string_view, chess::MAX_ARGS> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    const auto programName = args.front();

    args = args.subspan(1uz);

    if (args.empty()
        or std::ranges::contains(args, "--help")) {
        chess::print_help(programName);
        return EXIT_FAILURE;
    }

    chess::run_perft(chess::parse_options(args));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
