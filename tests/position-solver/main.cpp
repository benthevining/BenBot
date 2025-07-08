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
#include <iostream>
#include <iterator>
#include <libbenbot/search/Search.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
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

    namespace search = chess::search;

    search::Context context {
        search::Callbacks {
            .onSearchComplete = [&context](const search::Callbacks::Result& result) {
                std::println("{}",
                    chess::notation::to_alg(context.options.position, result.bestMove));
            } }
    };

    context.options.position = chess::notation::from_fen(fenString);
    context.options.depth    = chess::util::int_from_string(depthString, 4uz);

    context.search();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
