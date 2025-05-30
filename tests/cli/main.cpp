/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <algorithm>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using chess::game::Position;

struct GameOptions final {
    Position startingPosition {};

    bool uciMoveFormat { false };
};

namespace {

void print_help(const std::string_view programName)
{
    std::println("Usage:");
    std::println("{} [--fen \"<fenString>\"] [--uci]", programName);
    std::println("The --uci flag tells the program to parse input moves in the UCI format. If this flag is not given, SAN format is the default.");
}

[[nodiscard]] GameOptions parse_options(std::span<const std::string_view> args)
{
    GameOptions options {};

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

        if (arg == "--uci") {
            options.uciMoveFormat = true;
        }
    };

    return options;
}

using chess::game::print_utf8;

void game_loop(Position position, const bool uciMoveFormat)
{
    std::string nextMove;

    do {
        std::println("{}", print_utf8(position));

    read_next_move:
        std::println("{} to move:",
            magic_enum::enum_name(position.sideToMove));

        nextMove.clear();

        std::cin >> nextMove;

        try {
            if (uciMoveFormat) {
                const auto move = chess::notation::from_uci(position, nextMove);

                std::println("{}", chess::notation::to_uci(move));

                position.make_move(move);
            } else {
                const auto move = chess::notation::from_alg(position, nextMove);

                std::println("{}", chess::notation::to_alg(position, move));

                position.make_move(move);
            }
        } catch (const std::invalid_argument& exception) {
            std::println("{}", exception.what());
            goto read_next_move;
        }

        const bool anyLegalMoves = ! chess::moves::generate(position).empty();

        if (position.is_check()) {
            if (anyLegalMoves)
                std::println("Check!");
            else {
                std::println("Checkmate!");
                break;
            }
        } else if (! anyLegalMoves) {
            std::println("Stalemate!");
            break;
        }
    } while (true);

    std::println("{}", print_utf8(position));
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

    if (std::ranges::contains(args, "--help")) {
        print_help(programName);
        return EXIT_FAILURE;
    }

    const auto [startingPosition, uciMoveFormat] = parse_options(args);

    game_loop(startingPosition, uciMoveFormat);

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
