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
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/util/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace chess {

using std::println;

struct PerftOptions final {
    game::Position startingPosition;

    std::size_t depth { 1uz };

    std::optional<std::filesystem::path> jsonOutputPath;
};

namespace {

    struct PerftResult final {
        size_t nodes { 0uz };

        size_t captures { 0uz };

        size_t enPassantCaptures { 0uz };

        size_t castles { 0uz };

        size_t promotions { 0uz };

        size_t checks { 0uz };

        size_t checkmates { 0uz };

        size_t stalemates { 0uz };

        std::vector<std::pair<moves::Move, size_t>> rootNodes;

        PerftResult& operator+=(const PerftResult& rhs) noexcept;
    };

    PerftResult& PerftResult::operator+=(const PerftResult& rhs) noexcept
    {
        nodes += rhs.nodes;
        captures += rhs.captures;
        enPassantCaptures += rhs.enPassantCaptures;
        castles += rhs.castles;
        promotions += rhs.promotions;
        checks += rhs.checks;
        checkmates += rhs.checkmates;
        stalemates += rhs.stalemates;

        return *this;
    }

    void print_help(const std::string_view programName)
    {
        println("Usage:");
        println("{} <depth> [--fen \"<fenString>\"] [--write-json <path>]", programName);
        println("If the --write-json option is given, a JSON file with results will be written to the given path.");
    }

    [[nodiscard]] PerftOptions parse_options(std::span<const std::string_view> args)
    {
        PerftOptions options {};

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
        const PerftOptions& options,
        const PerftResult&  result,
        const auto          wallTime)
    {
        if (! options.jsonOutputPath.has_value())
            return;

        const auto& path = *options.jsonOutputPath;

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

        std::filesystem::create_directories(path.parent_path());

        std::ofstream output { path };

        output << json.dump(1);

        println("Wrote JSON results to {}", path.string()); // NOLINT(build/include_what_you_use)
        println("");
    }

    void print_root_nodes(const PerftResult& result)
    {
        for (const auto [move, numChildren] : result.rootNodes) {
            println("{} {}",
                notation::to_uci(move), numChildren);
        }
    }

    void print_results(
        const PerftResult& result,
        const auto         wallTime)
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

    template <bool IsRoot = true>
    [[nodiscard]] PerftResult perft(const size_t depth, const game::Position& startingPosition = {}) // NOLINT(readability-function-cognitive-complexity)
    {
        if (depth == 0uz)
            return { .nodes = 1uz };

        PerftResult result;

        for (const auto& move : moves::generate(startingPosition)) {
            const auto newPosition = after_move(startingPosition, move);

            // we want stats only for leaf nodes
            if (depth == 1uz) {
                if (startingPosition.is_capture(move)) {
                    ++result.captures;

                    if (startingPosition.is_en_passant(move))
                        ++result.enPassantCaptures;
                }

                if (move.is_castling())
                    ++result.castles;

                if (move.promotedType.has_value())
                    ++result.promotions;

                const bool isCheck = newPosition.is_check();

                if (isCheck)
                    ++result.checks;

                if (! moves::any_legal_moves(newPosition)) {
                    if (isCheck)
                        ++result.checkmates;
                    else
                        ++result.stalemates;
                }
            }

            const auto childResult = perft<false>(depth - 1uz, newPosition);

            if constexpr (IsRoot) {
                result.rootNodes.emplace_back(move, childResult.nodes);
            }

            result += childResult;
        }

        return result;
    }

    void run_perft(const PerftOptions& options)
    {
        using Clock = std::chrono::high_resolution_clock;

        println("Starting position:");
        println("{}", game::print_utf8(options.startingPosition));
        println("Running perft depth {}...", options.depth);
        println("");

        const auto startTime = Clock::now();

        const auto result = perft(options.depth, options.startingPosition);

        const auto endTime = Clock::now();

        const auto wallTime = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);

        write_json_file(options, result, wallTime);

        print_root_nodes(result);

        println("");

        print_results(result, wallTime);
    }

} // namespace

} // namespace chess

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
        chess::print_help(programName);
        return EXIT_FAILURE;
    }

    chess::run_perft(chess::parse_options(args));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
