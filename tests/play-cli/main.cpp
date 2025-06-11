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
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <libchess/game/Position.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/util/Strings.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using chess::game::Position;
using chess::moves::Move;
using chess::notation::GameRecord;
using chess::pieces::Color;
using std::size_t;

void print_help(const std::string_view programName)
{
    std::println("Usage:");
    std::println(
        "{} [--fen <startingFEN>] [--white|--black] [--alg|--uci] [--depth <N>] [--pgn <outFile>]",
        programName);
}

struct Options final {
    Position startingPosition;

    Color computerPlays { Color::Black };

    bool useUCI { false };

    size_t searchDepth { 4uz };

    std::optional<std::filesystem::path> pgnOut;
};

[[nodiscard]] Options parse_options(std::span<const std::string_view> args)
{
    Options opts {};

    while (! args.empty()) {
        const auto arg = args.front();

        args = args.subspan(1uz);

        if (arg == "--fen") {
            if (args.empty()) {
                throw std::invalid_argument { "Expected FEN string after argument --fen" };
            }

            opts.startingPosition = chess::notation::from_fen(args.front());

            args = args.subspan(1uz);

            continue;
        }

        if (arg == "--depth") {
            if (args.empty()) {
                throw std::invalid_argument { "Expected depth value after argument --depth" };
            }

            const auto depthStr = args.front();

            args = args.subspan(1uz);

            opts.searchDepth = chess::util::int_from_string(depthStr, opts.searchDepth);

            continue;
        }

        if (arg == "--pgn") {
            if (args.empty()) {
                throw std::invalid_argument { "Expected filepath after argument --pgn" };
            }

            opts.pgnOut = args.front();

            args = args.subspan(1uz);

            continue;
        }

        if (arg == "--white") {
            opts.computerPlays = Color::White;
            continue;
        }

        if (arg == "--black") {
            opts.computerPlays = Color::Black;
            continue;
        }

        if (arg == "--alg") {
            opts.useUCI = false;
            continue;
        }

        if (arg == "--uci") {
            opts.useUCI = true;
            continue;
        }

        throw std::invalid_argument {
            std::format("Unrecognized argument '{}'", arg)
        };
    }

    return opts;
}

struct CLIGame final {
    explicit CLIGame(Options opts)
        : options { std::move(opts) }
    {
        gameRecord.startingPosition = options.startingPosition;
    }

    void loop()
    {
        while (! currentPosition.get_result().has_value()) {
            std::println("{}", chess::game::print_utf8(currentPosition));

            const auto move = currentPosition.sideToMove == options.computerPlays
                                ? get_computer_move()
                                : read_user_move();

            currentPosition.make_move(move);

            gameRecord.moves.push_back(GameRecord::Move { .move = move });
        }

        gameRecord.result = currentPosition.get_result();

        print_result();

        write_pgn_file();
    }

private:
    [[nodiscard]] Move get_computer_move() const
    {
        std::println("Computer is thinking...");

        const auto move = chess::search::find_best_move(currentPosition, options.searchDepth);

        if (options.useUCI) {
            std::println("{} plays: {}",
                magic_enum::enum_name(options.computerPlays), chess::notation::to_uci(move));
        } else {
            std::println("{} plays: {}",
                magic_enum::enum_name(options.computerPlays), chess::notation::to_alg(currentPosition, move));
        }

        return move;
    }

    [[nodiscard]] Move read_user_move()
    {
        std::println("{} to play:",
            magic_enum::enum_name(currentPosition.sideToMove));

        inputBuf.clear();

        std::cin >> inputBuf;

        try {
            if (options.useUCI)
                return chess::notation::from_uci(currentPosition, inputBuf);

            return chess::notation::from_alg(currentPosition, inputBuf);
        } catch (const std::invalid_argument& exception) {
            std::println("{}", exception.what());

            return read_user_move(); // try again
        }
    }

    void print_result() const
    {
        switch (gameRecord.result.value()) {
            using enum chess::game::Result;

            case Draw:
                std::println("Draw!");
                break;

            case WhiteWon:
                std::println("White wins!");
                break;

            case BlackWon:
                std::println("Black wins!");
        }
    }

    void write_pgn_file() const
    {
        if (! options.pgnOut.has_value())
            return;

        const auto pgnPath = options.pgnOut.value();

        std::filesystem::create_directories(pgnPath.parent_path());

        std::ofstream stream { pgnPath };

        stream << chess::notation::to_pgn(gameRecord);

        std::println("Wrote PGN file to {}", pgnPath.string());
    }

    Options options;

    std::string inputBuf;

    Position currentPosition { options.startingPosition };

    GameRecord gameRecord;
};

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

    CLIGame game { parse_options(args) };

    game.loop();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
