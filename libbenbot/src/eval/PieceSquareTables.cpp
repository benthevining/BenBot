/*
 * ======================================================================================
 *
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
 * ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
 *
 * ======================================================================================
 */

#include "PieceSquareTables.hpp" // NOLINT(build/include_subdir)
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <numeric>
#include <span>
#include <utility>

namespace ben_bot::eval {

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
    -10,  6,   0,   0,   0,   0,   6,  -10,
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

// KING: Middle-game
// Make the king stand behind the pawn shelter
// Harsh penalties for king dragged out into the open
static constexpr std::array kingMiddlegameTable {
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

// KING: Endgame
static constexpr std::array kingEndgameTable {
//   A1
    -50, -30, -30, -30, -30, -30, -30, -50,
    -30, -30,  0,   0,   0,   0,  -30, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -20, -10,  0,   0,  -10, -20, -30,
    -50, -40, -30, -20, -20, -30, -40, -50
//                                      H8
};

// clang-format on

namespace {

    using chess::board::Pieces;

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr int sum_squares(
        chess::board::Bitboard board, const std::span<const int> table) noexcept
    {
        if constexpr (IsBlack) {
            board = chess::board::flips::vertical(board);
        }

        const auto indices = board.indices();

        return std::transform_reduce(
            indices.begin(), indices.end(),
            0,
            std::plus {},
            [table](const auto idx) { return table[idx]; });
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr int score_pawns(const Pieces& pieces)
    {
        return sum_squares<IsBlack>(pieces.pawns, pawnTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr int score_knights(const Pieces& pieces)
    {
        return sum_squares<IsBlack>(pieces.knights, knightTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr int score_bishops(const Pieces& pieces)
    {
        return sum_squares<IsBlack>(pieces.bishops, bishopTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr int score_rooks(const Pieces& pieces)
    {
        return sum_squares<IsBlack>(pieces.rooks, rookTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr int score_queens(const Pieces& pieces)
    {
        return sum_squares<IsBlack>(pieces.queens, queenTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] int score_king(
        const Pieces& pieces, const float endgameWeight)
    {
        auto king = pieces.king;

        assert(king.count() == 1uz);

        if constexpr (IsBlack) {
            king = chess::board::flips::vertical(king);
        }

        const auto idx = king.first();

        const auto middlegameValue = kingMiddlegameTable.at(idx);
        const auto endgameValue    = kingEndgameTable.at(idx);

        return static_cast<int>(std::round(static_cast<float>(middlegameValue) * (1.f - endgameWeight)))
             + static_cast<int>(std::round(static_cast<float>(endgameValue) * endgameWeight));
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] int score_side_pieces(
        const Pieces& pieces, const float endgameWeight)
    {
        return score_pawns<IsBlack>(pieces)
             + score_knights<IsBlack>(pieces)
             + score_bishops<IsBlack>(pieces)
             + score_rooks<IsBlack>(pieces)
             + score_queens<IsBlack>(pieces)
             + score_king<IsBlack>(pieces, endgameWeight);
    }

} // namespace

int score_piece_placement(const Position& position, const float endgameWeight)
{
    const auto [ourScore, theirScore] = [&position, endgameWeight] {
        if (position.is_black_to_move()) {
            return std::make_pair(
                score_side_pieces<true>(position.our_pieces(), endgameWeight),
                score_side_pieces<false>(position.their_pieces(), endgameWeight));
        }

        return std::make_pair(
            score_side_pieces<false>(position.our_pieces(), endgameWeight),
            score_side_pieces<true>(position.their_pieces(), endgameWeight));
    }();

    return ourScore - theirScore;
}

} // namespace ben_bot::eval
