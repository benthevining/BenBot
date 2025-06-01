/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

// TODO:

// GUI -> Engine:
// setoption name <id> [value <x>]

// Engine -> GUI
// info
// option

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using chess::game::Position;
using chess::moves::Move;

using chess::notation::from_uci;
using chess::util::trim;

namespace {

void print_engine_id()
{
    std::println("id name BenBot");
    std::println("id author Ben Vining");
}

[[nodiscard]] std::pair<std::string_view, std::string_view>
split_at_first_space(
    const std::string_view input)
{
    const auto spaceIdx = input.find(' ');

    if (spaceIdx == std::string_view::npos) {
        return { input, {} };
    }

    return {
        input.substr(0uz, spaceIdx),
        input.substr(spaceIdx + 1uz)
    };
}

struct GoCommandOptions final {
    std::vector<Move> moves; // restrict search to only these moves

    bool ponderMode { false }; // this means that lastMove is the ponder move

    size_t whiteMsLeft { 0uz };
    size_t blackMsLeft { 0uz };

    size_t whiteIncMs { 0uz };
    size_t blackIncMs { 0uz };

    size_t movesToGo { 0uz }; // moves to the next time control

    size_t depth { 0uz }; // search this many plies only

    size_t nodes { 0uz }; // search this many nodes only

    size_t mate { 0uz }; // search for a mate in x moves

    size_t searchTime { 0uz }; // search exactly x mseconds

    bool infinite { false }; // search until the "stop" command
};

[[nodiscard]] GoCommandOptions parse_go_command_options(
    std::string_view args, const Position& currentPosition)
{
    // args doesn't include the "go" token itself

    GoCommandOptions options;

    while (! args.empty()) {
        auto [firstWord, rest] = split_at_first_space(args);

        firstWord = trim(firstWord);

        args = rest;

        if (firstWord == "ponder") {
            options.ponderMode = true;
            continue;
        }

        if (firstWord == "infinite") {
            options.infinite = true;
            continue;
        }

        if (firstWord == "wtime") {
            auto [wtime, rest2] = split_at_first_space(rest);

            wtime = trim(wtime);

            args = rest2;

            std::from_chars(
                wtime.data(), wtime.data() + wtime.length(), options.whiteMsLeft);

            continue;
        }

        if (firstWord == "btime") {
            auto [btime, rest2] = split_at_first_space(rest);

            btime = trim(btime);

            args = rest2;

            std::from_chars(
                btime.data(), btime.data() + btime.length(), options.blackMsLeft);

            continue;
        }

        if (firstWord == "winc") {
            auto [winc, rest2] = split_at_first_space(rest);

            winc = trim(winc);

            args = rest2;

            std::from_chars(
                winc.data(), winc.data() + winc.length(), options.whiteIncMs);

            continue;
        }

        if (firstWord == "binc") {
            auto [binc, rest2] = split_at_first_space(rest);

            binc = trim(binc);

            args = rest2;

            std::from_chars(
                binc.data(), binc.data() + binc.length(), options.blackIncMs);

            continue;
        }

        if (firstWord == "movestogo") {
            auto [movesToGo, rest2] = split_at_first_space(rest);

            movesToGo = trim(movesToGo);

            args = rest2;

            std::from_chars(
                movesToGo.data(), movesToGo.data() + movesToGo.length(), options.movesToGo);

            continue;
        }

        if (firstWord == "depth") {
            auto [depth, rest2] = split_at_first_space(rest);

            depth = trim(depth);

            args = rest2;

            std::from_chars(
                depth.data(), depth.data() + depth.length(), options.depth);

            continue;
        }

        if (firstWord == "nodes") {
            auto [nodes, rest2] = split_at_first_space(rest);

            nodes = trim(nodes);

            args = rest2;

            std::from_chars(
                nodes.data(), nodes.data() + nodes.length(), options.nodes);

            continue;
        }

        if (firstWord == "mate") {
            auto [mate, rest2] = split_at_first_space(rest);

            mate = trim(mate);

            args = rest2;

            std::from_chars(
                mate.data(), mate.data() + mate.length(), options.mate);

            continue;
        }

        if (firstWord == "movetime") {
            auto [movetime, rest2] = split_at_first_space(rest);

            movetime = trim(movetime);

            args = rest2;

            std::from_chars(
                movetime.data(), movetime.data() + movetime.length(), options.searchTime);

            continue;
        }

        if (firstWord != "searchmoves")
            continue;

        // searchmoves

        using std::operator""sv;

        static constexpr std::array argumentTokens {
            "searchmoves"sv, "ponder"sv, "wtime"sv, "btime"sv, "winc"sv, "binc"sv,
            "movestogo"sv, "depth"sv, "nodes"sv, "mate"sv, "movetime"sv, "infinite"sv
        };

        while (! args.empty()) {
            auto [firstMove, rest2] = split_at_first_space(rest);

            firstMove = trim(firstMove);

            if (std::ranges::contains(argumentTokens, firstMove))
                break;

            args = rest2;

            options.moves.emplace_back(from_uci(currentPosition, firstMove));
        }
    }

    return options;
}

[[nodiscard]] std::random_device& get_rng_seed()
{
    static std::random_device rngSeed;
    return rngSeed;
}

} // namespace

struct UCIEngine final {
    void loop()
    {
        do {
            read_next_command();
        } while (! shouldExit);
    }

private:
    void read_next_command()
    {
        inputBuf.clear();

        std::cin >> inputBuf;

        handle_command(inputBuf);
    }

    void handle_command(std::string_view command)
    {
        command = trim(command);

        if (command.empty())
            return;

        if (command == "uci") {
            // this command is sent once after program boot
            print_engine_id();
            std::println("uciok");
            return;
        }

        if (command == "isready") {
            // can block here to wait for background tasks to complete
            std::println("readyok");
            return;
        }

        if (command == "ucinewgame") {
            currentPosition = Position {};
            // flush any search-related data structures, etc.
            // isready will be queried after this
            return;
        }

        if (command == "quit") {
            shouldExit = true; // exit the event loop
            return;
        }

        if (command == "stop") {
            // interrupt the search
            return;
        }

        if (command == "ponderhit") {
            // the engine was told to ponder on the same move the user has played
            // the engine should continue searching but switch from pondering to normal search
            return;
        }

        auto [firstWord, rest] = split_at_first_space(command);

        firstWord = trim(firstWord);

        if (firstWord == "position") {
            handle_position_command(rest);
            return;
        }

        if (firstWord == "go") {
            handle_go_command(rest);
            return;
        }

        if (firstWord == "setoption") {
            handle_set_option(rest);
            return;
        }

        if (firstWord == "debug") {
            rest      = trim(rest);
            debugMode = rest == "on";
        }
    }

    void handle_position_command(const std::string_view args)
    {
        // position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
        // args doesn't include the "position" token itself

        auto [secondWord, rest] = split_at_first_space(args);

        secondWord = trim(secondWord);

        if (secondWord == "startpos") {
            currentPosition = Position {};
        } else if (secondWord == "fen") {
            const auto [fenString, afterFEN] = split_at_first_space(rest);

            currentPosition = chess::notation::from_fen(fenString);

            rest = afterFEN;
        }

        auto [moveToken, moves] = split_at_first_space(rest);

        moveToken = trim(moveToken);

        if (moveToken != "moves")
            return;

        while (! moves.empty()) {
            const auto [firstMove, rest2] = split_at_first_space(moves);

            lastMove = from_uci(currentPosition, firstMove);

            currentPosition.make_move(lastMove);

            moves = rest2;
        }
    }

    void handle_go_command(const std::string_view args)
    {
        // args does not include the "go" token itself
        [[maybe_unused]] const auto opts = parse_go_command_options(args, currentPosition);

        // run search with given options...
        std::println("bestmove {}",
            chess::notation::to_uci(pick_best_move()));
    }

    void handle_set_option(const std::string_view args)
    {
        // args does not include the "setoption" token itself

        // TODO
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

    Position currentPosition {};

    Move lastMove {};

    std::string inputBuf;

    bool shouldExit { false };

    bool debugMode { false };

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

    UCIEngine engine;

    engine.loop();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
