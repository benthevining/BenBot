/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <array>
#include <cassert>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/eval/PieceSquareTables.hpp>
#include <libchess/pieces/Colors.hpp>
#include <utility>

namespace chess::eval {

// clang-format off

// see https://www.chessprogramming.org/Simplified_Evaluation_Function

// PAWNS
// Shelter in front of castling locations (b1, c1, d1 and f1, g1, h1) get bonuses
// Negative value for f3, smaller penalty for g3
// h2 has same value as h3, so this advance is allowed, e.g. to chase off a pinning bishop
// 0 value on f4, g4, h4 discourages these pawns from stopping here - either keep them back or go all in on a pawn storm
// Negative values for central pawns on starting rank incentivizes them to be pushed, preferably 2 squares
// Beginning with 6th rank we give bonuses for advanced pawns. On 7th rank even bigger bonus.
static constexpr std::array pawnTable {
//  A1
    0,  0,   0,   0,   0,   0,   0,   0,
    5,  10,  10, -20, -20,  10,  10,  5,
    5, -5,  -10,  0,   0,  -10, -5,   5,
    0,  0,   0,   20,  20,  0,   0,   0,
    5,  5,   10,  25,  25,  10,  5,   5,
    10, 10,  20,  30,  30,  20,  10,  10,
    50, 50,  50,  50,  50,  50,  50,  50,
    0,  0,   0,   0,   0,   0,   0,   0
//                                    H8
};

// KNIGHTS
// Simply encourage them to go to the center and discourage the perimeter
static constexpr std::array knightTable {
//  A1
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,  0,   5,   5,   0,  -20, -40,
    -30,  5,   10,  15,  15,  10,  5,  -30,
    -30,  0,   15,  20,  20,  15,  0,  -30,
    -30,  5,   15,  20,  20,  15,  5,  -30,
    -30,  0,   10,  15,  15,  10,  0,  -30,
    -40, -20,  5,   10,  10,  5,  -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
//                                      H8
};

// BISHOPS
// Similar to knights, we discourage perimeter squares, but the penalty isn't as big
// Additionally we prefer squares like b3, c4, b5, d3 and the central ones
static constexpr std::array bishopTable {
//  A1
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,  5,   0,   0,   0,   0,   5,  -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,  0,   10,  10,  10,  10,  0,  -10,
    -10,  5,   5,   10,  10,  5,   5,  -10,
    -10,  0,   5,   10,  10,  5,   0,  -10,
    -10,  0,   0,   0,   0,   0,   0,  -10,
    -20, -10, -10, -10, -10, -10, -10, -20
//                                      H8
};

// ROOKS
// Bonuses given to centralize & occupy the 7th rank
// Avoid A & H files
static constexpr std::array rookTable {
//  A1
     0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5,  10, 10, 10, 10, 10, 10, 5,
     0,  0,  0,  0,  0,  0,  0,  0
//                               H8
};

// QUEENS
// Avoid perimeter, and especially corners
// Slight bonus for centralizing, also b3 & c2
static constexpr std::array queenTable {
//  A1
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10,  0,   5,   0,  0,  0,   0,  -10,
    -10,  5,   5,   5,  5,  5,   0,  -10,
     0,   0,   5,   5,  5,  5,   0,  -5,
    -5,   0,   5,   5,  5,  5,   0,  -5,
    -10,  0,   5,   5,  5,  5,   0,  -10,
    -10,  0,   0,   0,  0,  0,   0,  -10,
    -20, -10, -10, -5, -5, -10, -10, -20
//                                    H8
};

// KING
// For now these are middle game values
// TODO: separate table for endgame values
// Make the king stand behind the pawn shelter
// Harsh penalties for king dragged out into the open
static constexpr std::array kingTable {
//   A1
     20,  30,  10,  0,   0,   10,  30,  20,
     20,  20,  0,   0,   0,   0,   20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30
//                                      H8
};

// clang-format on

namespace {

    using board::Bitboard;

    template <bool IsBlack>
    [[nodiscard, gnu::const]] Value score_side_pieces(
        const board::Pieces& pieces) noexcept
    {
        using board::flips::vertical;

        Value score { 0 };

        {
            auto pawns = pieces.pawns;

            if constexpr (IsBlack) {
                pawns = vertical(pawns);
            }

            for (const auto idx : pawns.indices())
                score += pawnTable[idx]; // cppcheck-suppress useStlAlgorithm
        }
        {
            auto knights = pieces.knights;

            if constexpr (IsBlack) {
                knights = vertical(knights);
            }

            for (const auto idx : knights.indices())
                score += knightTable[idx]; // cppcheck-suppress useStlAlgorithm
        }
        {
            auto bishops = pieces.bishops;

            if constexpr (IsBlack) {
                bishops = vertical(bishops);
            }

            for (const auto idx : bishops.indices())
                score += bishopTable[idx]; // cppcheck-suppress useStlAlgorithm
        }
        {
            auto rooks = pieces.rooks;

            if constexpr (IsBlack) {
                rooks = vertical(rooks);
            }

            for (const auto idx : rooks.indices())
                score += rookTable[idx]; // cppcheck-suppress useStlAlgorithm
        }
        {
            auto queens = pieces.queens;

            if constexpr (IsBlack) {
                queens = vertical(queens);
            }

            for (const auto idx : queens.indices())
                score += queenTable[idx]; // cppcheck-suppress useStlAlgorithm
        }
        {
            auto king = pieces.king;

            assert(king.count() == 1uz);

            if constexpr (IsBlack) {
                king = vertical(king);
            }

            score += kingTable[king.first()];
        }

        return score;
    }

} // namespace

Value score_piece_placement(const Position& position) noexcept
{
    const auto [ourScore, theirScore] = [&position] {
        if (position.sideToMove == pieces::Color::Black) {
            return std::make_pair(
                score_side_pieces<true>(position.our_pieces()),
                score_side_pieces<false>(position.their_pieces()));
        }

        return std::make_pair(
            score_side_pieces<false>(position.our_pieces()),
            score_side_pieces<true>(position.their_pieces()));
    }();

    return ourScore - theirScore;
}

} // namespace chess::eval
