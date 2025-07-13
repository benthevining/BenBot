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

#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <exception>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <libbenbot/search/Search.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/util/Files.hpp>
#include <libchess/util/Strings.hpp>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using std::size_t;

int main(const int argc, const char** argv)
try {
    namespace notation = chess::notation;
    namespace search   = ben_bot::search;

    const std::vector<std::string_view> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    const auto programName = args.front();

    args = args.subspan(1uz);

    if (args.empty()) {
        std::println("Usage:");
        std::println("{} <epdPath>", programName);
        return EXIT_FAILURE;
    }

    std::optional<chess::moves::Move> foundMove;

    search::Context context {
        search::Callbacks {
            .onSearchComplete = [&foundMove](const search::Callbacks::Result& result) {
                foundMove = result.bestMove;
            } }
    };

    const auto fileContent = chess::util::load_file_as_string(
        std::filesystem::path { args.front() });

    size_t numPassed { 0uz };
    size_t numFailed { 0uz };

    for (const auto& epd : notation::parse_all_epds(fileContent)) {
        context.options.position = epd.position;

        // clear this so that all legal moves in the position will be searched
        context.options.movesToSearch.clear();

        context.options.depth = chess::util::int_from_string(
            epd.operations.at("depth"), 4uz);

        const auto expectedMove = notation::from_alg(
            epd.position, epd.operations.at("bm"));

        foundMove.reset();

        context.clear_transposition_table();

        context.search();

        if (foundMove.value() == expectedMove) {
            ++numPassed;
            continue;
        }

        ++numFailed;

        std::println(std::cerr,
            "Position failed: {} ({})",
            notation::to_fen(epd.position),
            epd.operations.at("comment"));

        std::println(std::cerr,
            "Expected {}, got {}",
            notation::to_alg(epd.position, expectedMove),
            notation::to_alg(epd.position, *foundMove));
    }

    std::println(
        "{} test cases passed, {} test cases failed",
        numPassed, numFailed);

    return numFailed;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
