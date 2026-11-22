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

#include <libbenbot/eval/TaperedScore.hpp>
#include <libchess/board/Bitboard.hpp>

namespace chess::game {
struct Position;
} // namespace chess::game

namespace ben_bot {
class PawnTable;
} // namespace ben_bot

namespace ben_bot::eval::detail {

using chess::game::Position;

// the pawn structure terms that only depend on the positions of the pawns
struct PawnTerms final {
    // the sum of all pawn-only terms, from White's point of view
    TaperedScore whiteScore;

    // both sides' passed pawns
    chess::board::Bitboard passers;
};

// calculates all the pawn structure terms that only depend on the pawns
[[nodiscard, gnu::pure]] auto score_pawns_only(const Position& position) -> PawnTerms;

// calculates the passed pawn terms that depend on the rooks & kings, from White's point of view
[[nodiscard, gnu::pure]] auto score_passer_piece_terms(
    const Position& position, chess::board::Bitboard passers) noexcept
    -> TaperedScore;

// returns the pawn structure score from the side to move's point of view
// if pawnTable isn't null, the pawn-only terms are looked up in it (or stored in it on a miss)
[[nodiscard]] auto score_pawn_structure(const Position& position, PawnTable* pawnTable) -> TaperedScore;

} // namespace ben_bot::eval::detail
