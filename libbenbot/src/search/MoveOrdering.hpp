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

#include <libchess/moves/Move.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>
#include <span>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace ben_bot {
class TranspositionTable;
struct HistoryTable;
struct ContinuationHistory;
} // namespace ben_bot

namespace ben_bot::search::detail {

using chess::game::Position;
using chess::moves::Move;

// ttMove is the best move stored in the transposition table for currentPosition, if any, & is
// ordered first. prevMove is the move that led to currentPosition, & ownPrevMove is the move 1
// ply before that (played by the side to move). Either is a null move if there isn't one
void order_moves_for_search(
    const Position&            currentPosition,
    std::span<Move>            moves,
    const TranspositionTable&  transTable,
    std::optional<Move>        ttMove,
    std::span<const Move>      killerMoves,
    const HistoryTable&        history,
    const ContinuationHistory& continuationHistory,
    Move                       prevMove,
    Move                       ownPrevMove);

void order_moves_for_q_search(
    const Position&     currentPosition,
    std::span<Move>     moves,
    const HistoryTable& history);

// returns the type of piece the given capture takes, which is a pawn for en passant
[[nodiscard]] auto captured_type(const Position& currentPosition, Move move) noexcept
    -> chess::pieces::Type;

} // namespace ben_bot::search::detail
