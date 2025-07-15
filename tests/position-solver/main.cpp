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

#include <beman/inplace_vector/inplace_vector.hpp>
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
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using std::size_t;

namespace {

constexpr auto MAX_ARGS = 3uz;

void print_usage(const std::string_view programName)
{
    std::println("Usage:");
    std::println("{} <epdPath> <lineIdx>", programName);
}

[[nodiscard]] auto get_line(
    const std::string_view text, const size_t lineIdx) -> std::string_view
{
    const auto lines = chess::util::lines_view(text)
                     | std::ranges::to<std::vector>();

    return lines.at(lineIdx);
}

} // namespace

int main(const int argc, const char** argv)
try {
    namespace notation = chess::notation;
    namespace search   = ben_bot::search;

    const beman::inplace_vector<std::string_view, MAX_ARGS> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    const auto programName = args.front();

    args = args.subspan(1uz);

    if (args.empty()) {
        print_usage(programName);
        return EXIT_FAILURE;
    }

    const auto fileContent = chess::util::load_file_as_string(
        std::filesystem::path { args.front() });

    args = args.subspan(1uz);

    if (args.empty()) {
        print_usage(programName);
        return EXIT_FAILURE;
    }

    const auto lineIdx = chess::util::int_from_string<size_t>(args.front());

    const auto epd = notation::from_epd(
        get_line(fileContent, lineIdx));

    std::optional<chess::moves::Move> foundMove;

    search::Context context {
        search::Callbacks {
            .onSearchComplete = [&foundMove](const search::Callbacks::Result& result) {
                foundMove = result.bestMove;
            } }
    };

    context.options.position = epd.position;

    context.options.depth = chess::util::int_from_string(
        epd.operations.at("depth"), 4uz);

    const auto expectedMove = notation::from_alg(
        epd.position, epd.operations.at("bm"));

    context.search();

    if (foundMove.value() == expectedMove) {
        std::println("Passed!");
        return EXIT_SUCCESS;
    }

    std::println(std::cerr,
        "Position failed: {} ({})",
        notation::to_fen(epd.position),
        epd.operations.at("id"));

    std::println(std::cerr,
        "Expected {}, got {}",
        notation::to_alg(epd.position, expectedMove),
        notation::to_alg(epd.position, *foundMove));

    return EXIT_FAILURE;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
