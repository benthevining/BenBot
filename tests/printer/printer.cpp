/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/game/Position.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <print>

int main()
{
    static constexpr chess::board::Square starting { chess::board::File::D, chess::board::Rank::Four };

    static constexpr auto moves = chess::moves::pseudo_legal::queen(starting);

    std::println("{}", chess::board::print_ascii(moves));

    return 0;

    using chess::game::print_utf8;

    chess::game::Position position;

    std::println("{}", print_utf8(position));

    std::println("");

    position.make_move({ .from = { chess::board::File::E, chess::board::Rank::Two },
        .to                    = { chess::board::File::E, chess::board::Rank::Four },
        .piece                 = chess::pieces::Type::Pawn });

    std::println("{}", print_utf8(position));
}
