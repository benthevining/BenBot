/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <libchess/game/Position.hpp>
#include <libchess/moves/Legal.hpp>
#include <print>

int main()
{
    using chess::board::Bitboard;
    using chess::board::File;
    using chess::board::Rank;
    using chess::board::Square;

    static constexpr Square starting { File::D, Rank::Four };

    Bitboard occupied;

    occupied.set(Square { File::C, Rank::Three });
    occupied.set(Square { File::F, Rank::Six });

    Bitboard friendly;

    friendly.set(Square { File::F, Rank::Six });

    std::println("{}", chess::board::print_ascii(chess::moves::legal::bishop(starting, occupied, friendly)));

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
