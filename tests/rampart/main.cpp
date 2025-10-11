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

// This executable is invoked with a single positional arguments: a FEN starting position.
// The executable writes JSON output containing every legal move in the given starting position,
// as well as the FEN after making the move.

#include <beman/inplace_vector/inplace_vector.hpp>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <nlohmann/json.hpp>
#include <print>
#include <span>
#include <string_view>

int main(const int argc, const char** argv)
try {
    static constexpr auto MAX_ARGS = 3uz;

    const beman::inplace_vector::inplace_vector<std::string_view, MAX_ARGS> argStorage {
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

    nlohmann::json json;

    json["startPos"] = fenString;

    auto movesJSON = nlohmann::json::array();

    const auto position = chess::notation::from_fen(fenString);

    if (not position.has_value()) {
        std::println(std::cerr, "{}", position.error());
        return EXIT_FAILURE;
    }

    for (const auto& move : chess::moves::generate(position.value())) {
        nlohmann::json moveJSON;

        moveJSON["move"] = chess::notation::to_alg(position.value(), move);

        moveJSON["fen"] = chess::notation::to_fen(
            after_move(position.value(), move),
            false);

        movesJSON.push_back(moveJSON);
    }

    json["generated"] = movesJSON;

    std::println("{}", json.dump(1));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
