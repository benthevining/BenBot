/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <iterator>
#include <libchess/eval/Evaluation.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/search/Search.hpp>
#include <print>
#include <span>
#include <string_view>
#include <vector>

int main(const int argc, const char** argv)
{
    const std::vector<std::string_view> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    // program name
    args = args.subspan(1uz);

    const auto position = chess::notation::from_fen(args.front());

    std::println("{}",
        chess::game::print_utf8(position));

    std::println("Eval: {}",
        chess::eval::evaluate(position));

    std::println("Best move: {}",
        chess::notation::to_alg(position,
            chess::search::find_best_move(position)));
}
