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
#include <libchess/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>
#include <print>
#include <random>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using chess::game::Position;
using chess::pieces::Color;

struct GameOptions final {
    Position startingPosition {};

    bool uciMoveFormat { false };

    Color computerPlays { Color::Black };
};

namespace {

void print_help(const std::string_view programName)
{
    std::println("Usage:");
    std::println("{} [--fen \"<fenString>\"] [--uci] [--white]", programName);
    std::println("The --uci flag tells the program to parse input moves in the UCI format. If this flag is not given, SAN format is the default.");
    std::println("The --white flag tells the computer to play the White pieces. If this flag is not given, the computer plays Black by default.");
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
            continue;
        }

        if (arg == "--white") {
            options.computerPlays = Color::White;
        }
    };

    return options;
}

using chess::game::print_utf8;
using chess::moves::Move;

[[nodiscard]] Move pick_best_move(const Position& position)
{
    static std::random_device rngSeed;

    static std::mt19937 rng { rngSeed() };

    auto moves = chess::moves::generate(position);

    // make the bot a bit more interesting to play against
    std::ranges::shuffle(moves, rng);

    const auto evals = moves
                     | std::views::transform([position](const Move& move) {
                           return chess::eval::evaluate(chess::game::after_move(position, move));
                       })
                     | std::ranges::to<std::vector>();

    // evals are in range [0, 1], so find worst score for our opponent -> best score for us
    const auto minScore = std::ranges::min_element(evals);

    const auto minScoreIdx = std::ranges::distance(evals.begin(), minScore);

    return moves.at(minScoreIdx);
}

[[nodiscard]] Move read_user_move(
    const Position& position, std::string& userInputBuf, const bool uciMoveFormat)
{
read_next_move:

    std::println("{} to move:",
        magic_enum::enum_name(position.sideToMove));

    userInputBuf.clear();

    std::cin >> userInputBuf;

    try {
        if (uciMoveFormat)
            return chess::notation::from_uci(position, userInputBuf);

        return chess::notation::from_alg(position, userInputBuf);
    } catch (const std::invalid_argument& exception) {
        std::println("{}", exception.what());
        goto read_next_move;
    }
}

[[nodiscard]] Move get_next_move(
    const GameOptions& options, const Position& position, std::string& userInputBuf)
{
    if (position.sideToMove == options.computerPlays)
        return pick_best_move(position);

    return read_user_move(position, userInputBuf, options.uciMoveFormat);
}

void game_loop(const GameOptions& options)
{
    Position position { options.startingPosition };

    std::string userInputBuf;

    do {
        std::println("{}", print_utf8(position));

        const auto move = get_next_move(options, position, userInputBuf);

        if (options.uciMoveFormat)
            std::println("{}", chess::notation::to_uci(move));
        else
            std::println("{}", chess::notation::to_alg(position, move));

        position.make_move(move);

        const bool anyLegalMoves = chess::moves::any_legal_moves(position);

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

    game_loop(parse_options(args));

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
