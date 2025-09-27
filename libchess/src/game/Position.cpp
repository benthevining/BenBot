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

#include "Zobrist.hpp"
#include <array>
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <format>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <string>
#include <utility>

namespace chess::game {

using board::Rank;
using std::size_t;
using std::uint_least8_t;

namespace {

    void update_bitboards(
        Position& position, const Move& move) noexcept
    {
        const bool isWhite = position.is_white_to_move();

        auto& ourPieces      = position.our_pieces();
        auto& opponentPieces = position.their_pieces();

        opponentPieces.capture_at(move.to());

        ourPieces.our_move(move, position.sideToMove);

        if (position.is_en_passant(move)) {
            [[unlikely]];

            const auto idx = board::get_en_passant_captured_square(
                position.enPassantTargetSquare.value(), isWhite)
                                 .index();

            opponentPieces.pawns.unset(idx);
            opponentPieces.occupied.unset(idx);
        }
    }

    [[nodiscard, gnu::const]] constexpr auto get_en_passant_target_square(
        const Move& move, const bool isWhite) noexcept
        -> std::optional<Square>
    {
        if (move.piece() != PieceType::Pawn
            or std::cmp_not_equal(rank_distance(move.from(), move.to()), 2uz)) {
            [[likely]];
            return std::nullopt;
        }

        return Square {
            .file = move.to().file,
            .rank = isWhite ? Rank::Three : Rank::Six
        };
    }

    [[nodiscard]] constexpr auto update_castling_rights(
        Position& pos, const bool isWhite, const Move& move) noexcept
        -> zobrist::CastlingRightsChanges
    {
        const auto whiteOldRights { pos.whiteCastlingRights };
        const auto blackOldRights { pos.blackCastlingRights };

        auto& ourRights   = isWhite ? pos.whiteCastlingRights : pos.blackCastlingRights;
        auto& theirRights = isWhite ? pos.blackCastlingRights : pos.whiteCastlingRights;

        ourRights.our_move(move);

        if (isWhite)
            theirRights.their_move<Color::Black>(move);
        else
            theirRights.their_move<Color::White>(move);

        const auto& whiteNewRights = pos.whiteCastlingRights;
        const auto& blackNewRights = pos.blackCastlingRights;

        return {
            .whiteKingside  = whiteOldRights.kingside != whiteNewRights.kingside,
            .whiteQueenside = whiteOldRights.queenside != whiteNewRights.queenside,
            .blackKingside  = blackOldRights.kingside != blackNewRights.kingside,
            .blackQueenside = blackOldRights.queenside != blackNewRights.queenside
        };
    }

} // namespace

void Position::make_move(const Move& move)
{
    const bool isCapture = is_capture(move);
    const bool isWhite   = is_white_to_move();

    const auto newEPSquare = get_en_passant_target_square(move, isWhite);

    const auto rightsChanges = update_castling_rights(*this, isWhite, move);

    hash = zobrist::update(*this, move, newEPSquare, rightsChanges);

    update_bitboards(*this, move);

    if (move.piece() == PieceType::Pawn || isCapture) {
        // reset half-move counter (for 50-move draws & threefold repetition)
        halfmoveClock = UINT8_C(0);
        threefoldChecker.reset(hash);
    } else {
        ++halfmoveClock;
        threefoldChecker.push(hash);
    }

    enPassantTargetSquare = newEPSquare;

    // increment full move counter after every Black move
    if (not isWhite)
        ++fullMoveCounter;

    // flip side to move
    sideToMove = isWhite ? Color::Black : Color::White;
}

void Position::make_null_move()
{
    const bool isWhite = is_white_to_move();

    hash = zobrist::after_null_move(*this);

    ++halfmoveClock;
    threefoldChecker.push(hash);

    enPassantTargetSquare = std::nullopt;

    // increment full move counter after every Black move
    if (not isWhite)
        ++fullMoveCounter;

    // flip side to move
    sideToMove = isWhite ? Color::Black : Color::White;
}

namespace {
    using board::Square;

    [[nodiscard, gnu::const, gnu::cold]] constexpr auto square_vertical_flip(const Square starting) noexcept -> Square
    {
        const auto board = board::Bitboard::from_square(starting);

        return Square::from_index(
            board::flips::vertical(board).first());
    }
} // namespace

void Position::flip()
{
    whitePieces = std::exchange(blackPieces, whitePieces);

    whitePieces.vertical_flip();
    blackPieces.vertical_flip();

    sideToMove = is_white_to_move() ? Color::Black : Color::White;

    whiteCastlingRights = std::exchange(blackCastlingRights, whiteCastlingRights);

    enPassantTargetSquare = enPassantTargetSquare.and_then([](const Square oldEPSquare) {
        return std::optional { square_vertical_flip(oldEPSquare) };
    });

    refresh_zobrist();
}

Position::Position()
{
    hash = zobrist::calculate(*this); // NOLINT(cppcoreguidelines-prefer-member-initializer)

    threefoldChecker.reset(hash);
}

void Position::refresh_zobrist()
{
    hash = zobrist::calculate(*this);

    threefoldChecker.reset(hash);
}

auto Position::is_checkmate() const -> bool
{
    return is_check()
       and not moves::any_legal_moves(*this);
}

auto Position::is_stalemate() const -> bool
{
    return not is_check()
       and not moves::any_legal_moves(*this);
}

auto Position::is_fifty_move_draw() const -> bool
{
    return std::cmp_greater_equal(halfmoveClock, 100)
       and moves::any_legal_moves(*this); // side to move must have at least 1 legal move
}

auto Position::is_draw_by_insufficient_material() const noexcept -> bool
{
    // even if either side has a single pawn that can't move, mate can still be possible
    if (whitePieces.pawns.any() or blackPieces.pawns.any()
        or whitePieces.rooks.any() or blackPieces.rooks.any()
        or whitePieces.queens.any() or blackPieces.queens.any()) {
        [[likely]];
        return false;
    }

    const auto numWhiteKnights = whitePieces.knights.count();
    const auto numWhiteBishops = whitePieces.bishops.count();

    const auto numBlackKnights = blackPieces.knights.count();
    const auto numBlackBishops = blackPieces.bishops.count();

    const bool whiteHasOnlyKing = numWhiteKnights + numWhiteBishops == 0uz;
    const bool blackHasOnlyKing = numBlackKnights + numBlackBishops == 0uz;

    if (not(whiteHasOnlyKing or blackHasOnlyKing)) {
        [[likely]];
        return false;
    }

    if (whiteHasOnlyKing and blackHasOnlyKing)
        return true;

    // check if side without the lone king has only 1 knight/bishop

    if (whiteHasOnlyKing)
        return numBlackKnights + numBlackBishops == 1uz;

    return numWhiteKnights + numWhiteBishops == 1uz;
}

auto Position::is_draw() const -> bool
{
    if (is_threefold_repetition() or is_draw_by_insufficient_material())
        return true;

    if (moves::any_legal_moves(*this))
        return std::cmp_greater_equal(halfmoveClock, 100); // fifty-move draw

    return not is_check(); // stalemate
}

auto Position::get_result() const -> std::optional<Result>
{
    if (is_draw())
        return Result::Draw;

    if (not is_checkmate())
        return std::nullopt;

    if (is_white_to_move()) {
        // White to move and we're checkmated, Black won
        return Result::BlackWon;
    }

    return Result::WhiteWon;
}

auto Position::is_illegal() const -> std::optional<std::string>
{
    // for reference, see the rules defined at https://github.com/lechmazur/ChessCounter#rules

    // each side must have exactly 1 king
    {
        if (const auto whiteKings = whitePieces.king.count();
            whiteKings != 1uz) {
            return std::format(
                "White has {} kings, expected 1", whiteKings);
        }

        if (const auto blackKings = blackPieces.king.count();
            blackKings != 1uz) {
            return std::format(
                "Black has {} kings, expected 1", blackKings);
        }
    }

    // no pawns on 1st or 8th rank
    {
        static constexpr auto outerRanksMask = board::masks::ranks::ONE | board::masks::ranks::EIGHT;

        if (const auto pawns = whitePieces.pawns | blackPieces.pawns;
            (pawns & outerRanksMask).any()) {
            return "Pawns found on 1st or 8th rank";
        }
    }

    // at most 16 pieces per side
    {
        static constexpr auto MAX_PIECES = 16uz;

        if (const auto numWhite = whitePieces.occupied.count();
            numWhite > MAX_PIECES) {
            return std::format(
                "White has {} pieces, expected at most {}",
                numWhite, MAX_PIECES);
        }

        if (const auto numBlack = blackPieces.occupied.count();
            numBlack > MAX_PIECES) {
            return std::format(
                "Black has {} pieces, expected at most {}",
                numBlack, MAX_PIECES);
        }
    }

    // at most 8 pawns, 9 queens, 10 bishops, 10 rooks, 10 knights per side
    {
        using PieceInfo = std::pair<PieceType, size_t>;

        static constexpr std::array PIECES_INFO {
            PieceInfo { PieceType::Pawn, 8uz },
            PieceInfo { PieceType::Queen, 9uz },
            PieceInfo { PieceType::Bishop, 10uz },
            PieceInfo { PieceType::Rook, 10uz },
            PieceInfo { PieceType::Knight, 10uz }
        };

        for (const auto& [type, maxNum] : PIECES_INFO) {
            if (const auto numWhite = whitePieces.get_type(type).count();
                numWhite > maxNum) {
                return std::format(
                    "White has {} {}s, expected at most {}",
                    numWhite, type, maxNum);
            }

            if (const auto numBlack = blackPieces.get_type(type).count();
                numBlack > maxNum) {
                return std::format(
                    "Black has {} {}s, expected at most {}",
                    numBlack, type, maxNum);
            }
        }
    }

    // if the other side's king is in check when it's our move, the position is illegal
    if (const auto otherColor = is_white_to_move() ? Color::Black : Color::White;
        is_side_in_check(otherColor)) {
        return std::format(
            "{} is in check during {}'s move",
            magic_enum::enum_name(otherColor),
            magic_enum::enum_name(sideToMove));
    }

    return std::nullopt;
}

} // namespace chess::game
