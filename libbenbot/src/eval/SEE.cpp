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

#include <algorithm>
#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/eval/Material.hpp>
#include <libbenbot/eval/SEE.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <optional>

namespace ben_bot::eval {

namespace {

    using chess::board::Bitboard;
    using chess::board::Pieces;
    using chess::board::Rank;
    using chess::board::Square;
    using chess::pieces::Color;

    namespace magics   = chess::moves::magics;
    namespace patterns = chess::moves::patterns;

    [[nodiscard]] auto diagonal_sliders(const Position& position) noexcept -> Bitboard
    {
        return position.whitePieces.bishops | position.whitePieces.queens
             | position.blackPieces.bishops | position.blackPieces.queens;
    }

    [[nodiscard]] auto orthogonal_sliders(const Position& position) noexcept -> Bitboard
    {
        return position.whitePieces.rooks | position.whitePieces.queens
             | position.blackPieces.rooks | position.blackPieces.queens;
    }

    // returns the pieces of both colors that attack the target square, given the occupancy
    [[nodiscard]] auto attackers_to(
        const Position& position, const Square target, const Bitboard occupied)
        -> Bitboard
    {
        const auto targetBoard = Bitboard::from_square(target);

        const auto& white = position.whitePieces;
        const auto& black = position.blackPieces;

        // a white pawn attacks the target square if a black pawn on the target square would attack it
        const auto pawns = (patterns::pawn_attacks<Color::Black>(targetBoard) & white.pawns)
                         | (patterns::pawn_attacks<Color::White>(targetBoard) & black.pawns);

        const auto knights = patterns::knight(targetBoard) & (white.knights | black.knights);
        const auto kings   = patterns::king(targetBoard) & (white.king | black.king);

        const auto bishops = magics::bishop(target, occupied, { }) & diagonal_sliders(position);
        const auto rooks   = magics::rook(target, occupied, { }) & orthogonal_sliders(position);

        return (pawns | knights | kings | bishops | rooks) & occupied;
    }

    struct Attacker final {
        PieceType type { PieceType::Pawn };
        Bitboard  square;
    };

    [[nodiscard]] auto least_valuable_attacker(
        const Pieces& pieces, const Bitboard attackers) noexcept
        -> std::optional<Attacker>
    {
        static constexpr std::array types {
            PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
            PieceType::Rook, PieceType::Queen, PieceType::King
        };

        for (const auto type : types) {
            if (const auto subset = attackers & pieces.get_type(type); subset.any()) {
                return Attacker {
                    .type   = type,
                    .square = Bitboard::from_square(Square::from_index(subset.first()))
                };
            }
        }

        return std::nullopt;
    }

} // namespace

auto see(const Position& position, const Move move) -> int
{
    // castling can't capture anything, and the rook & king always land on safe squares
    if (move.is_castling())
        return 0;

    const auto target = move.to();

    const bool targetIsBackRank = target.rank == Rank::One or target.rank == Rank::Eight;

    // gains[i] is the net material won by the side making the i-th capture
    // in the exchange, assuming that capture is the last one. There can
    // be at most 32 captures, since there are at most 32 pieces on the board.
    std::array<int, 32uz> gains { };

    auto occupied = position.occupied() & Bitboard::from_square(move.from()).inverse();

    if (position.is_en_passant(move)) {
        gains.front() = piece_values::PAWN;

        const auto capturedSquare = chess::board::get_en_passant_captured_square(
            target, position.is_white_to_move());

        occupied &= Bitboard::from_square(capturedSquare).inverse();
    } else if (const auto captured = position.their_pieces().get_piece_on(target)) {
        gains.front() = piece_values::get(*captured);
    }

    // value of the piece currently standing on the target square, which can be captured next
    int onTarget = piece_values::get(move.piece());

    if (const auto promoted = move.promoted_type()) {
        gains.front() += piece_values::get(*promoted) - piece_values::PAWN;

        onTarget = piece_values::get(*promoted);
    }

    auto attackers = attackers_to(position, target, occupied);

    bool whiteToCapture = not position.is_white_to_move();

    auto depth = 0uz;

    while (true) {
        const auto& side = whiteToCapture ? position.whitePieces : position.blackPieces;

        const auto attacker = least_valuable_attacker(side, attackers);

        if (not attacker.has_value())
            break;

        ++depth;

        gains.at(depth) = onTarget - gains.at(depth - 1uz);

        // capturing the king ends the exchange. The king's side will never allow this
        // when backing up the gains, so this is how illegal king captures are handled
        if (onTarget == piece_values::KING)
            break;

        onTarget = piece_values::get(attacker->type);

        // a pawn recapturing on the back rank promotes (we assume to a queen)
        if (attacker->type == PieceType::Pawn and targetIsBackRank) {
            gains.at(depth) += piece_values::QUEEN - piece_values::PAWN;

            onTarget = piece_values::QUEEN;
        }

        occupied &= attacker->square.inverse();

        // moving the attacker may have uncovered X-ray attackers behind it. Knights
        // can't stand on a line through the target square, so they can't block one
        if (attacker->type != PieceType::Knight) {
            attackers |= magics::bishop(target, occupied, { }) & diagonal_sliders(position);
            attackers |= magics::rook(target, occupied, { }) & orthogonal_sliders(position);
        }

        attackers &= occupied;

        whiteToCapture = not whiteToCapture;
    }

    // each side may decline to make its capture, if doing so would lose material
    for (; depth > 0uz; --depth) {
        gains.at(depth - 1uz) = -std::max(-gains.at(depth - 1uz), gains.at(depth));
    }

    return gains.front();
}

} // namespace ben_bot::eval
