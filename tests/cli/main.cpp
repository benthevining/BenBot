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

[[nodiscard]] std::random_device& get_rng_seed()
{
    static std::random_device rngSeed;
    return rngSeed;
}

} // namespace

struct ConsoleGame final {
    explicit ConsoleGame(const GameOptions& options)
        : currentPosition { options.startingPosition }
        , uciMoveFormat { options.uciMoveFormat }
        , computerPlays { options.computerPlays }
    {
    }

    void loop()
    {
        using chess::game::print_utf8;

        do {
            std::println("{}", print_utf8(currentPosition));

            const bool anyLegalMoves = chess::moves::any_legal_moves(currentPosition);

            if (currentPosition.is_check()) {
                if (anyLegalMoves)
                    std::println("Check!");
                else {
                    std::println("Checkmate!");
                    return;
                }
            } else if (! anyLegalMoves) {
                std::println("Stalemate.");
                return;
            }

            const auto move = get_next_move();

            if (uciMoveFormat)
                std::println("{}", chess::notation::to_uci(move));
            else
                std::println("{}", chess::notation::to_alg(currentPosition, move));

            currentPosition.make_move(move);
        } while (true);
    }

private:
    using Move = chess::moves::Move;

    [[nodiscard]] Move get_next_move()
    {
        if (currentPosition.sideToMove == computerPlays)
            return pick_best_move();

        return read_user_move();
    }

    [[nodiscard]] Move pick_best_move()
    {
        auto moves = chess::moves::generate(currentPosition);

        // make the bot a bit more interesting to play against
        std::ranges::shuffle(moves, rng);

        const auto evals = moves
                         | std::views::transform([this](const Move& move) {
                               return chess::eval::evaluate(
                                   chess::game::after_move(currentPosition, move));
                           })
                         | std::ranges::to<std::vector>();

        // evals are in range [0, 1], so find worst score for our opponent -> best score for us
        const auto minScore = std::ranges::min_element(evals);

        const auto minScoreIdx = std::ranges::distance(evals.begin(), minScore);

        return moves.at(minScoreIdx);
    }

    [[nodiscard]] Move read_user_move() // NOLINT(misc-no-recursion)
    {
        std::println("{} to move:",
            magic_enum::enum_name(currentPosition.sideToMove));

        userInputBuf.clear();

        std::cin >> userInputBuf;

        try {
            if (uciMoveFormat)
                return chess::notation::from_uci(currentPosition, userInputBuf);

            return chess::notation::from_alg(currentPosition, userInputBuf);
        } catch (const std::invalid_argument& exception) {
            std::println("{}", exception.what());
            return read_user_move(); // try again
        }
    }

    Position currentPosition;

    bool uciMoveFormat { false };

    Color computerPlays { Color::Black };

    std::string userInputBuf;

    std::mt19937 rng { get_rng_seed()() };
};

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

    ConsoleGame game { parse_options(args) };

    game.loop();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
