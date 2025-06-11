/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/game/Result.hpp>
#include <libchess/game/Zobrist.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/UTF8.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace chess::game {

using board::Rank;

namespace {

    // given the en passant target square, this returns the square that the
    // captured pawn was on
    [[nodiscard, gnu::const]] Square get_en_passant_captured_square(
        const Square& targetSquare, const bool isWhite) noexcept
    {
        // the captured pawn is on the file of the target square, but
        // one rank below (White capture) or one rank above (Black capture)
        const auto capturedRank = isWhite
                                    ? board::prev_pawn_rank<Color::White>(targetSquare.rank)
                                    : board::prev_pawn_rank<Color::Black>(targetSquare.rank);

        return Square {
            .file = targetSquare.file,
            .rank = capturedRank
        };
    }

    void update_bitboards(
        Position& position, const Move& move) noexcept
    {
        const bool isWhite = position.sideToMove == Color::White;

        auto& ourPieces      = position.our_pieces();
        auto& opponentPieces = position.their_pieces();

        opponentPieces.capture_at(move.to);

        ourPieces.our_move(move, position.sideToMove);

        if (position.is_en_passant(move)) {
            [[unlikely]];

            const auto idx = get_en_passant_captured_square(
                position.enPassantTargetSquare.value(), isWhite)
                                 .index();

            opponentPieces.pawns.unset(idx);
            opponentPieces.occupied.unset(idx);
        }
    }

    [[nodiscard, gnu::const]] std::optional<Square> get_en_passant_target_square(
        const Move& move, const bool isWhite) noexcept
    {
        if (move.piece != PieceType::Pawn
            || std::cmp_not_equal(board::rank_distance(move.from, move.to), 2uz)) {
            [[likely]];
            return std::nullopt;
        }

        return Square {
            .file = move.to.file,
            .rank = isWhite ? Rank::Three : Rank::Six
        };
    }

    // each of these bools are true if the given right has changed since the last move
    struct CastlingRightsChanges final {
        bool whiteKingside { false };
        bool whiteQueenside { false };
        bool blackKingside { false };
        bool blackQueenside { false };
    };

    [[nodiscard]] CastlingRightsChanges update_castling_rights(
        Position& pos, const bool isWhite, const Move& move) noexcept
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

    [[nodiscard, gnu::const]] std::uint_least8_t tick_halfmove_clock(
        const Move& move, const bool isCapture, const std::uint_least8_t prevValue) noexcept
    {
        if (isCapture || (move.piece == PieceType::Pawn))
            return 0;

        static constexpr auto MAX_VALUE = static_cast<std::uint_least8_t>(100);

        if (std::cmp_greater_equal(prevValue, MAX_VALUE)) {
            [[unlikely]];
            return MAX_VALUE;
        }

        return prevValue + 1;
    }

    [[nodiscard, gnu::const]] zobrist::Value update_zobrist(
        const Position& pos, const Move& move,
        const std::optional<Square>  newEPTarget,
        const CastlingRightsChanges& rightsChanges)
    {
        auto value = pos.hash;

        value ^= zobrist::BLACK_TO_MOVE; // just toggle these bits in/out every other move

        // remove old EP target
        if (pos.enPassantTargetSquare.has_value())
            value ^= zobrist::en_passant_key(pos.enPassantTargetSquare->file);

        // add new EP target
        if (newEPTarget.has_value())
            value ^= zobrist::en_passant_key(newEPTarget->file);

        // remove moved-from square
        value ^= zobrist::piece_key(move.piece, pos.sideToMove, move.from);

        // add moved-to square
        value ^= zobrist::piece_key(
            move.is_promotion() ? *move.promotedType : move.piece,
            pos.sideToMove, move.to);

        if (pos.is_capture(move)) {
            const auto otherColor = pos.sideToMove == Color::White ? Color::Black : Color::White;

            if (pos.is_en_passant(move)) {
                [[unlikely]];

                value ^= zobrist::piece_key(
                    PieceType::Pawn, otherColor,
                    get_en_passant_captured_square(
                        pos.enPassantTargetSquare.value(),
                        pos.sideToMove == Color::White));
            } else {
                [[likely]];

                const auto capturedType = pos.their_pieces().get_piece_on(move.to);

                value ^= zobrist::piece_key(
                    capturedType.value(), otherColor, move.to);
            }
        } else if (move.is_castling()) {
            [[unlikely]];
            if (move.to.is_kingside())
                value ^= board::masks::kingside_castle_rook_pos_mask(pos.sideToMove).to_int();
            else
                value ^= board::masks::queenside_castle_rook_pos_mask(pos.sideToMove).to_int();
        }

        if (rightsChanges.whiteKingside)
            value ^= zobrist::WHITE_KINGSIDE_CASTLE;

        if (rightsChanges.whiteQueenside)
            value ^= zobrist::WHITE_QUEENSIDE_CASTLE;

        if (rightsChanges.blackKingside)
            value ^= zobrist::BLACK_KINGSIDE_CASTLE;

        if (rightsChanges.blackQueenside)
            value ^= zobrist::BLACK_QUEENSIDE_CASTLE;

        return value;
    }

} // namespace

void Position::make_move(const Move& move)
{
    const bool isCapture = is_capture(move);
    const bool isWhite   = sideToMove == Color::White;

    const auto newEPSquare = get_en_passant_target_square(move, isWhite);

    const auto rightsChanges = update_castling_rights(*this, isWhite, move);

    hash = update_zobrist(*this, move, newEPSquare, rightsChanges);

    update_bitboards(*this, move);

    halfmoveClock = tick_halfmove_clock(move, isCapture, halfmoveClock);

    enPassantTargetSquare = newEPSquare;

    // increment full move counter after every Black move
    if (! isWhite)
        ++fullMoveCounter;

    // flip side to move
    sideToMove = isWhite ? Color::Black : Color::White;

    threefoldChecker.push(hash);
}

bool Position::is_checkmate() const
{
    return is_check()
        && ! moves::any_legal_moves(*this);
}

bool Position::is_stalemate() const
{
    return ! is_check()
        && ! moves::any_legal_moves(*this);
}

bool Position::is_fifty_move_draw() const
{
    return std::cmp_greater_equal(halfmoveClock, 100)
        && moves::any_legal_moves(*this); // side to move must have at least 1 legal move
}

bool Position::is_draw() const
{
    if (is_threefold_repetition())
        return true;

    if (moves::any_legal_moves(*this))
        return std::cmp_greater_equal(halfmoveClock, 100); // fifty-move draw

    return ! is_check(); // stalemate
}

std::optional<Result> Position::get_result() const
{
    if (is_draw())
        return Result::Draw;

    if (! is_checkmate())
        return std::nullopt;

    if (sideToMove == Color::White) {
        // White to move and we're checkmated, Black won
        return Result::BlackWon;
    }

    return Result::WhiteWon;
}

namespace utf8_pieces = pieces::utf8;

/* Example output of empty board:

    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |
    | | | | | | | | |

    A1 is bottom left, H8 is top right
 */

static constexpr std::string_view separator { "|" };

static constexpr std::string_view emptySquare { " |" };

std::string print_utf8(const Position& position)
{
    using board::Rank;
    using board::Square;

    const auto& whitePieces = position.whitePieces;
    const auto& blackPieces = position.blackPieces;

    std::string result;

    result.reserve(208uz);

    for (const auto rank : std::views::reverse(magic_enum::enum_values<Rank>())) {
        result.append(separator);

        for (const auto file : magic_enum::enum_values<File>()) {
            const Square square { .file = file, .rank = rank };

            if (const auto piece = whitePieces.get_piece_on(square)) {
                result.append(utf8_pieces::white::get(*piece));
                result.append(separator);
                continue;
            }

            if (const auto piece = blackPieces.get_piece_on(square)) {
                result.append(utf8_pieces::black::get(*piece));
                result.append(separator);
            } else {
                result.append(emptySquare);
            }
        }

        result.append("\n");
    }

    return result;
}

} // namespace chess::game
