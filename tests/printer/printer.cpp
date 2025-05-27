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
    using chess::board::Bitboard;
    using chess::board::File;
    using chess::board::Rank;
    using chess::board::Square;

    static constexpr Bitboard starting { Square { File::A, Rank::Eight } };

    Bitboard friendlyPieces;

    friendlyPieces.set(Square { File::A, Rank::Seven });
    friendlyPieces.set(Square { File::B, Rank::Seven });
    friendlyPieces.set(Square { File::C, Rank::Seven });

    const auto moves = chess::moves::pseudo_legal::king(starting, friendlyPieces);

    std::println("{}", chess::board::print_ascii(moves));

    std::println("{:0X}", moves.to_int());

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
