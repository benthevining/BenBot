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

#pragma once

#include <array>

namespace ben_bot::eval::pst {

// clang-format off

// see https://www.chessprogramming.org/Simplified_Evaluation_Function

// PAWNS
// Shelter in front of castling locations (b1, c1, d1 and f1, g1, h1) get bonuses
// Negative value for f3, smaller penalty for g3
// h2 has same value as h3, so this advance is allowed, e.g. to chase off a pinning bishop
// 0 value on f4, g4, h4 discourages these pawns from stopping here - either keep them back or go all in on a pawn storm
// Negative values for central pawns on starting rank incentivizes them to be pushed, preferably 2 squares
// Beginning with 6th rank we give bonuses for advanced pawns. On 7th rank even bigger bonus.
inline constexpr std::array pawnTable {
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
inline constexpr std::array knightTable {
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
inline constexpr std::array bishopTable {
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
inline constexpr std::array rookTable {
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
inline constexpr std::array queenTable {
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
inline constexpr std::array kingMiddlegameTable {
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
inline constexpr std::array kingEndgameTable {
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

} // namespace ben_bot::eval::pst
