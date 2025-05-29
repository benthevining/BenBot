/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <cstdlib>
#include <iostream>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/Algebraic.hpp>
#include <libchess/pieces/Colors.hpp>
#include <print>
#include <stdexcept>
#include <string>

// TODO: set starting FEN, print help

int main(const int argc, const char** argv)
{
    chess::game::Position position;

    std::string nextMove;

    do {
        std::println("{}", chess::game::print_utf8(position));

    read_next_move:
        const auto* colorString = position.sideToMove == chess::pieces::Color::White ? "White" : "Black";

        std::println("{} to move:", colorString);

        nextMove.clear();

        std::cin >> nextMove;

        try {
            const auto move = chess::notation::from_alg(position, nextMove);

            std::println("{}", chess::notation::to_alg(position, move));

            position.make_move(move);
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

    std::println("{}", chess::game::print_utf8(position));

    return EXIT_SUCCESS;
}
