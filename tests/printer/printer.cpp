/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/game/Position.hpp>
#include <libchess/moves/Legal.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <print>

int main()
{
    static constexpr chess::board::Square starting { chess::board::File::F, chess::board::Rank::Seven };

    static constexpr auto north = chess::moves::legal::detail::west_ray_attacks(starting,
        chess::board::Bitboard { chess::board::Square { chess::board::File::C, chess::board::Rank::Seven } });

    std::println("{}", chess::board::print_ascii(north));

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
