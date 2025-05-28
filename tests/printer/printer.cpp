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
    using chess::board::Pieces;
    using chess::pieces::Color;

    static constexpr Pieces whitePieces { Color::White };
    static constexpr Pieces blackPieces { Color::Black };

    static constexpr auto blackAttacks = attacked_squares<Color::Black>(blackPieces, whitePieces.occupied());

    std::println("{}", chess::board::print_ascii(blackAttacks));

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
