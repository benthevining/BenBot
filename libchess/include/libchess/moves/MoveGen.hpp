/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides move generation functions.
    @ingroup moves
 */

#pragma once

#include <array>
#include <cassert>
#include <iterator>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <ranges>
#include <vector>

namespace chess::moves {

using game::Position;
using PieceType = pieces::Type;

/// @ingroup moves
/// @{

/** Generates a list of all legal moves for the side to move in the given position.
    The list of moves is not sorted in any particular manner.

    @see generate_for()
 */
template <bool CapturesOnly = false>
void generate(
    const Position&                 position,
    std::output_iterator<Move> auto outputIt);

/** Generates a list of all legal moves for the side to move in the given position.
    If the side to move is in checkmate or stalemate, this returns an empty list.
    The list of moves is not sorted in any particular manner.

    @see generate_for()
 */
template <bool CapturesOnly = false>
[[nodiscard]] std::vector<Move> generate(const Position& position);

/** Generates a list of all legal moves for only the given piece type in the given position.

    Generating King moves will include castling. Generating pawn moves will include all
    pushes, double pushes, captures, promotions, and en passant captures.

    @see generate()
 */
template <bool CapturesOnly = false>
void generate_for(
    const Position& position, PieceType piece,
    std::output_iterator<Move> auto outputIt);

/** Generates a list of all legal moves for only the given piece type in the given position.

    Generating King moves will include castling. Generating pawn moves will include all
    pushes, double pushes, captures, promotions, and en passant captures.

    @see generate()
 */
template <bool CapturesOnly = false>
[[nodiscard]] std::vector<Move> generate_for(
    const Position& position, PieceType piece);

/** Returns true if the side to move has any legal moves in the given position. */
[[nodiscard]] bool any_legal_moves(const Position& position);

/// @}

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

namespace detail {

    using board::Bitboard;
    using board::Pieces;
    using board::Rank;
    using board::Square;
    using pieces::Color;

    using board::prev_pawn_rank;

    namespace rank_masks = board::masks::ranks;
    namespace shifts     = board::shifts;

    template <Color Side>
    static constexpr Color OtherSide = Side == Color::White ? Color::Black : Color::White;

    static constexpr auto promotionMask = rank_masks::ONE | rank_masks::EIGHT;

    static constexpr std::array possiblePromotedTypes {
        PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen
    };

    template <Color Side>
    constexpr void add_pawn_pushes(
        const Position& position, const Bitboard emptySquares,
        std::output_iterator<Move> auto outputIt)
    {
        const auto allPushes = pseudo_legal::pawn_pushes<Side>(
            position.pieces_for<Side>().pawns,
            emptySquares);

        // non-promoting pushes
        {
            const auto pushes = allPushes & promotionMask.inverse();

            for (const auto target : pushes.squares()) {
                const Move move {
                    .from = {
                        .file = target.file,
                        .rank = prev_pawn_rank<Side>(target.rank) },
                    .to    = target,
                    .piece = PieceType::Pawn
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }

        const auto promotions = allPushes & promotionMask;

        for (const auto target : promotions.squares()) {
            for (const auto promotedType : possiblePromotedTypes) {
                const Move move {
                    .from = {
                        .file = target.file,
                        .rank = prev_pawn_rank<Side>(target.rank) },
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side>
    constexpr void add_pawn_double_pushes(
        const Position& position, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        static constexpr auto pawnStartingRank = Side == Color::White ? Rank::Two : Rank::Seven;

        const auto pushes = pseudo_legal::pawn_double_pushes<Side>(
            position.pieces_for<Side>().pawns,
            allOccupied);

        for (const auto targetSquare : pushes.squares()) {
            const Move move {
                .from = {
                    .file = targetSquare.file,
                    .rank = pawnStartingRank },
                .to    = targetSquare,
                .piece = PieceType::Pawn
            };

            if (position.is_legal(move))
                *outputIt = move;
        }
    }

    template <Color Side>
    constexpr void add_pawn_captures(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        // We handle east & west captures separately to make set-wise operations easier.
        // This way, there is always a 1-1 relationship between a target square and a
        // starting square.

        const auto ourPawns = position.pieces_for<Side>().pawns;

        const auto enemyPieces = position.pieces_for<OtherSide<Side>>().occupied;

        const auto eastAttacks = shifts::pawn_capture_east<Side>(ourPawns);
        const auto westAttacks = shifts::pawn_capture_west<Side>(ourPawns);

        const auto eastCaptures = eastAttacks & enemyPieces;
        const auto westCaptures = westAttacks & enemyPieces;

        const auto eastPromotionCaptures = eastCaptures & promotionMask;
        const auto westPromotionCaptures = westCaptures & promotionMask;

        const auto eastRegCaptures = eastCaptures & promotionMask.inverse();
        const auto westRegCaptures = westCaptures & promotionMask.inverse();

        // starting positions of pawns that can make captures
        const auto canCapturePromoteEast = shifts::pawn_inv_capture_east<Side>(eastPromotionCaptures);
        const auto canCapturePromoteWest = shifts::pawn_inv_capture_west<Side>(westPromotionCaptures);

        const auto canRegCaptureEast = shifts::pawn_inv_capture_east<Side>(eastRegCaptures);
        const auto canRegCaptureWest = shifts::pawn_inv_capture_west<Side>(westRegCaptures);

        for (const auto [starting, target] : std::views::zip(canRegCaptureEast.squares(), eastRegCaptures.squares())) {
            const Move move {
                .from  = starting,
                .to    = target,
                .piece = PieceType::Pawn
            };

            if (position.is_legal(move))
                *outputIt = move;
        }

        for (const auto [starting, target] : std::views::zip(canRegCaptureWest.squares(), westRegCaptures.squares())) {
            const Move move {
                .from  = starting,
                .to    = target,
                .piece = PieceType::Pawn
            };

            if (position.is_legal(move))
                *outputIt = move;
        }

        for (const auto [starting, target] : std::views::zip(canCapturePromoteEast.squares(), eastPromotionCaptures.squares())) {
            for (const auto promotedType : possiblePromotedTypes) {
                const Move move {
                    .from         = starting,
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }

        for (const auto [starting, target] : std::views::zip(canCapturePromoteWest.squares(), westPromotionCaptures.squares())) {
            for (const auto promotedType : possiblePromotedTypes) {
                const Move move {
                    .from         = starting,
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side>
    constexpr void add_en_passant(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        if (! position.enPassantTargetSquare.has_value())
            return;

        const auto targetSquare = *position.enPassantTargetSquare;

        const auto targetSquareBoard = Bitboard::from_square(targetSquare);

        const auto startSquares = shifts::pawn_inv_capture_east<Side>(targetSquareBoard)
                                | shifts::pawn_inv_capture_west<Side>(targetSquareBoard);

        const auto ourPawns = position.pieces_for<Side>().pawns;

        const auto eligiblePawns = ourPawns & startSquares;

        for (const auto square : eligiblePawns.squares()) {
            const Move move {
                .from  = square,
                .to    = targetSquare,
                .piece = PieceType::Pawn
            };

            if (position.is_legal(move))
                *outputIt = move;
        }
    }

    template <Color Side, bool CapturesOnly>
    constexpr void add_all_pawn_moves(
        const Position& position, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        const auto ourPawns = position.pieces_for<Side>().pawns;

        if constexpr (! CapturesOnly) {
            add_pawn_pushes<Side>(position, allOccupied.inverse(), outputIt);
            add_pawn_double_pushes<Side>(position, allOccupied, outputIt);
        }

        add_pawn_captures<Side>(position, outputIt);
        add_en_passant<Side>(position, outputIt);
    }

    template <Color Side, bool CapturesOnly>
    constexpr void add_knight_moves(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        for (const auto knightPos : ourPieces.knights.subboards()) {
            auto knightMoves = pseudo_legal::knight(knightPos, ourPieces.occupied);

            if constexpr (CapturesOnly) {
                knightMoves &= position.pieces_for<OtherSide<Side>>().occupied;
            }

            for (const auto targetSquare : knightMoves.squares()) {
                const Move move {
                    .from  = Square::from_index(knightPos.first()),
                    .to    = targetSquare,
                    .piece = PieceType::Knight
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side, bool CapturesOnly>
    void add_bishop_moves(
        const Position&                 position,
        const Bitboard                  occupiedSquares,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        for (const auto bishopPos : ourPieces.bishops.squares()) {
            auto bishopMoves = magics::bishop(bishopPos, occupiedSquares, ourPieces.occupied);

            if constexpr (CapturesOnly) {
                bishopMoves &= position.pieces_for<OtherSide<Side>>().occupied;
            }

            for (const auto targetSquare : bishopMoves.squares()) {
                const Move move {
                    .from  = bishopPos,
                    .to    = targetSquare,
                    .piece = PieceType::Bishop
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side, bool CapturesOnly>
    void add_rook_moves(
        const Position&                 position,
        const Bitboard                  occupiedSquares,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        for (const auto rookPos : ourPieces.rooks.squares()) {
            auto rookMoves = magics::rook(rookPos, occupiedSquares, ourPieces.occupied);

            if constexpr (CapturesOnly) {
                rookMoves &= position.pieces_for<OtherSide<Side>>().occupied;
            }

            for (const auto targetSquare : rookMoves.squares()) {
                const Move move {
                    .from  = rookPos,
                    .to    = targetSquare,
                    .piece = PieceType::Rook
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side, bool CapturesOnly>
    void add_queen_moves(
        const Position&                 position,
        const Bitboard                  occupiedSquares,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        for (const auto queenPos : ourPieces.queens.squares()) {
            auto queenMoves = magics::queen(queenPos, occupiedSquares, ourPieces.occupied);

            if constexpr (CapturesOnly) {
                queenMoves &= position.pieces_for<OtherSide<Side>>().occupied;
            }

            for (const auto targetSquare : queenMoves.squares()) {
                const Move move {
                    .from  = queenPos,
                    .to    = targetSquare,
                    .piece = PieceType::Queen
                };

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side, bool CapturesOnly>
    constexpr void add_king_moves(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        auto kingMoves = pseudo_legal::king(ourPieces.king, ourPieces.occupied);

        if constexpr (CapturesOnly) {
            kingMoves &= position.pieces_for<OtherSide<Side>>().occupied;
        }

        const auto kingSquare = ourPieces.get_king_location();

        for (const auto targetSquare : kingMoves.squares()) {
            const Move move {
                .from  = kingSquare,
                .to    = targetSquare,
                .piece = PieceType::King
            };

            if (position.is_legal(move))
                *outputIt = move;
        }
    }

    // the two functions below are masks containing the set of squares that
    // must not be attacked/occupied in order for castling to be allowed

    template <Color Side>
    [[nodiscard, gnu::const]] consteval Bitboard kingside_castle_mask() noexcept
    {
        static constexpr auto rank = Side == Color::White ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { File::F, rank });
        board.set(Square { File::G, rank });

        return board;
    }

    // NB. with queenside castling, the set of squares that must be free/not attacked differ,
    // since castling is possible if the B1/B8 squares are attacked, but not if they are occupied
    template <Color Side, bool Occupied>
    [[nodiscard, gnu::const]] consteval Bitboard queenside_castle_mask() noexcept
    {
        static constexpr auto rank = Side == Color::White ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { File::C, rank });
        board.set(Square { File::D, rank });

        if constexpr (Occupied) {
            board.set(Square { File::B, rank });
        }

        return board;
    }

    template <Color Side>
    constexpr void add_castling(
        const Position& position, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        // castling out of check is not allowed
        if (position.is_check())
            return;

        static constexpr bool isWhite = Side == Color::White;

        const auto& rights = isWhite ? position.whiteCastlingRights : position.blackCastlingRights;

        if (! rights.either())
            return;

        static constexpr auto OppositeColor = isWhite ? Color::Black : Color::White;

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OppositeColor>();

        const auto allOurPieces = ourPieces.occupied;

        if (rights.kingside) {
            assert(ourPieces.rooks.test(Square { File::H, board::back_rank_for(position.sideToMove) }));

            static constexpr auto requiredSquares = kingside_castle_mask<Side>();

            const bool castlingBlocked = (requiredSquares & allOccupied).any()
                                      || squares_attacked<OppositeColor>(theirPieces, requiredSquares, allOurPieces);

            if (! castlingBlocked) {
                const auto move = castle_kingside(isWhite ? Color::White : Color::Black);

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }

        if (rights.queenside) {
            assert(ourPieces.rooks.test(Square { File::A, board::back_rank_for(position.sideToMove) }));

            static constexpr auto occupiedMask = queenside_castle_mask<Side, true>();
            static constexpr auto attackedMask = queenside_castle_mask<Side, false>();

            const bool castlingBlocked = (allOccupied & occupiedMask).any()
                                      || squares_attacked<OppositeColor>(theirPieces, attackedMask, allOurPieces);

            if (! castlingBlocked) {
                const auto move = castle_queenside(isWhite ? Color::White : Color::Black);

                if (position.is_legal(move))
                    *outputIt = move;
            }
        }
    }

    template <Color Side, bool CapturesOnly>
    void generate_internal(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OtherSide<Side>>();

        const auto allOccupied  = ourPieces.occupied | theirPieces.occupied;
        const auto emptySquares = allOccupied.inverse();

        add_all_pawn_moves<Side, CapturesOnly>(position, allOccupied, outputIt);

        add_knight_moves<Side, CapturesOnly>(position, outputIt);

        add_bishop_moves<Side, CapturesOnly>(position, allOccupied, outputIt);

        add_rook_moves<Side, CapturesOnly>(position, allOccupied, outputIt);

        add_queen_moves<Side, CapturesOnly>(position, allOccupied, outputIt);

        add_king_moves<Side, CapturesOnly>(position, outputIt);

        if constexpr (! CapturesOnly) {
            add_castling<Side>(position, allOccupied, outputIt);
        }
    }

    template <Color Side, bool CapturesOnly>
    void generate_for_internal(
        const Position& position, const PieceType piece,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OtherSide<Side>>();

        const auto allOccupied  = ourPieces.occupied | theirPieces.occupied;
        const auto emptySquares = allOccupied.inverse();

        switch (piece) {
            case PieceType::Pawn: {
                add_all_pawn_moves<Side, CapturesOnly>(position, allOccupied, outputIt);
                return;
            }

            case PieceType::Knight: {
                add_knight_moves<Side, CapturesOnly>(position, outputIt);
                return;
            }

            case PieceType::Bishop: {
                add_bishop_moves<Side, CapturesOnly>(position, allOccupied, outputIt);
                return;
            }

            case PieceType::Rook: {
                add_rook_moves<Side, CapturesOnly>(position, allOccupied, outputIt);
                return;
            }

            case PieceType::Queen: {
                add_queen_moves<Side, CapturesOnly>(position, allOccupied, outputIt);
                return;
            }

            default: // King
                add_king_moves<Side, CapturesOnly>(position, outputIt);

                if constexpr (! CapturesOnly) {
                    // castling is considered a King move
                    add_castling<Side>(position, allOccupied, outputIt);
                }
        }
    }

    template <Color Side>
    [[nodiscard]] bool any_legal_moves_internal(const Position& position)
    {
        std::vector<Move> moves;

        // as an optimization, check for king moves first, because in a double check,
        // a king move would be the only valid response

        for (const auto piece : { PieceType::King, PieceType::Pawn, PieceType::Knight, PieceType::Queen, PieceType::Rook, PieceType::Bishop }) {
            generate_for_internal<Side, false>(position, piece, std::back_inserter(moves));

            if (! moves.empty())
                return true;

            moves.clear();
        }

        return false;
    }

} // namespace detail

template <bool CapturesOnly>
void generate(
    const Position&                 position,
    std::output_iterator<Move> auto outputIt)
{
    if (position.sideToMove == Color::White)
        detail::generate_internal<Color::White, CapturesOnly>(position, outputIt);
    else
        detail::generate_internal<Color::Black, CapturesOnly>(position, outputIt);
}

template <bool CapturesOnly>
std::vector<Move> generate(const Position& position)
{
    std::vector<Move> moves;

    generate<CapturesOnly>(position, std::back_inserter(moves));

    return moves;
}

template <bool CapturesOnly>
void generate_for(
    const Position& position, const PieceType piece,
    std::output_iterator<Move> auto outputIt)
{
    if (position.sideToMove == Color::White)
        detail::generate_for_internal<Color::White, CapturesOnly>(position, piece, outputIt);
    else
        detail::generate_for_internal<Color::Black, CapturesOnly>(position, piece, outputIt);
}

template <bool CapturesOnly>
std::vector<Move> generate_for(
    const Position& position, const PieceType piece)
{
    std::vector<Move> moves;

    generate_for<CapturesOnly>(position, piece, std::back_inserter(moves));

    return moves;
}

inline bool any_legal_moves(const Position& position)
{
    if (position.sideToMove == Color::White)
        return detail::any_legal_moves_internal<Color::White>(position);

    return detail::any_legal_moves_internal<Color::Black>(position);
}

} // namespace chess::moves
