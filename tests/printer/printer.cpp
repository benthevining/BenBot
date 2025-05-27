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

    static constexpr Square starting { File::B, Rank::One };

    Bitboard friendlyPieces;

    friendlyPieces.set(Square { File::F, Rank::One });
    friendlyPieces.set(Square { File::G, Rank::Six });

    Bitboard enemyPieces;

    enemyPieces.set(Square { File::A, Rank::One });
    enemyPieces.set(Square { File::A, Rank::Two });
    enemyPieces.set(Square { File::B, Rank::Four });

    const auto moves = chess::moves::pseudo_legal::queen(
        starting, friendlyPieces | enemyPieces, friendlyPieces);

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
