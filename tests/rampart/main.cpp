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
#include <expected>
#include <iostream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <nlohmann/json.hpp>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace {
using nlohmann::json;

[[nodiscard]] auto run_rampart_test(const std::string_view fenString) -> std::expected<json, std::string>
{
    return chess::notation::from_fen(fenString)
        .transform([fenString](const chess::game::Position& position) {
            json obj;

            obj["startPos"] = fenString;

            auto movesJSON = json::array();

            for (const auto& move : chess::moves::generate(position)) {
                json moveJSON;

                moveJSON["move"] = chess::notation::to_alg(position, move);

                moveJSON["fen"] = chess::notation::to_fen(
                    after_move(position, move),
                    false);

                movesJSON.push_back(moveJSON);
            }

            obj["generated"] = movesJSON;

            return obj;
        });
}
} // namespace

int main(const int argc, const char** argv)
try {
    static constexpr auto MAX_ARGS = 2uz;

    if (std::cmp_greater(argc, MAX_ARGS)) {
        std::println(std::cerr,
            "Expected at most {} arguments, received {}",
            MAX_ARGS, argc);

        return EXIT_FAILURE;
    }

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

    [[maybe_unused]] const auto result
        = run_rampart_test(args.front())
              .transform([](const json& data) {
                  std::println("{}", data.dump(1));
              })
              .transform_error([](const std::string_view error) {
                  std::println(std::cerr, "Error: {}", error);
                  return std::monostate { };
              });

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
