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

#include "eval/PieceSquareTables.hpp"
#include "eval/PSTData.hpp"
#include <cassert>
#include <functional>
#include <libchess/board/Flips.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/game/Position.hpp>
#include <numeric>
#include <span>
#include <utility>

namespace ben_bot::eval {

namespace {

    using chess::board::Pieces;

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto sum_squares(
        chess::board::Bitboard board, const std::span<const int> table) noexcept
        -> int
    {
        if constexpr (IsBlack) {
            board = chess::board::flips::vertical(board);
        }

        const auto indices = board.indices();

        return std::transform_reduce(
            indices.begin(), indices.end(),
            0,
            std::plus {},
            [table](const auto idx) {
                assert(idx < table.size());
                return table[idx];
            });
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_pawns(const Pieces& pieces) -> int
    {
        return sum_squares<IsBlack>(pieces.pawns, pst::pawnTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_knights(const Pieces& pieces) -> int
    {
        return sum_squares<IsBlack>(pieces.knights, pst::knightTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_bishops(const Pieces& pieces) -> int
    {
        return sum_squares<IsBlack>(pieces.bishops, pst::bishopTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_rooks(const Pieces& pieces) -> int
    {
        return sum_squares<IsBlack>(pieces.rooks, pst::rookTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] constexpr auto score_queens(const Pieces& pieces) -> int
    {
        return sum_squares<IsBlack>(pieces.queens, pst::queenTable);
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] auto score_king(
        const Pieces& pieces, const float endgameWeight)
        -> int
    {
        auto king = pieces.king;

        assert(king.count() == 1uz);

        if constexpr (IsBlack) {
            king = chess::board::flips::vertical(king);
        }

        const auto idx = king.first();

        const auto middlegameValue = pst::kingMiddlegameTable.at(idx);
        const auto endgameValue    = pst::kingEndgameTable.at(idx);

        return static_cast<int>(std::round(static_cast<float>(middlegameValue) * (1.f - endgameWeight)))
             + static_cast<int>(std::round(static_cast<float>(endgameValue) * endgameWeight));
    }

    template <bool IsBlack>
    [[nodiscard, gnu::const]] auto score_side_pieces(
        const Pieces& pieces, const float endgameWeight)
        -> int
    {
        return score_pawns<IsBlack>(pieces)
             + score_knights<IsBlack>(pieces)
             + score_bishops<IsBlack>(pieces)
             + score_rooks<IsBlack>(pieces)
             + score_queens<IsBlack>(pieces)
             + score_king<IsBlack>(pieces, endgameWeight);
    }

} // namespace

auto score_piece_placement(
    const Position& position, const float endgameWeight)
    -> int
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
