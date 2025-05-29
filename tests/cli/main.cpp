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
#include <print>
#include <string>

// TODO: set starting FEN

int main(const int argc, const char** argv)
{
    chess::game::Position position;

    std::string nextMove;

    do {
        std::println("{}", chess::game::print_utf8(position));

        nextMove.clear();

        std::cin >> nextMove;

        const auto move = chess::notation::from_alg(position, nextMove);

        std::println("{}", chess::notation::to_alg(position, move));

        position.make_move(move);

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
