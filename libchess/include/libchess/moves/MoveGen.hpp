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
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/Pieces.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <ranges>
#include <utility>
#include <vector>

namespace chess::moves {

using game::Position;
using PieceType = pieces::Type;

/** Generates a list of all legal moves for the side to move in the given position.
    If the side to move is in checkmate or stalemate, this returns an empty list.

    @tparam PruneIllegal If true (the default), this function will return only strictly
    legal moves. If false, all pseudo-legal moves will be returned; that is, moves
    that obey the piece's movement mechanics, but may leave the side to move's king
    in check.

    @ingroup moves
    @see generate_for()
 */
template <bool PruneIllegal = true>
[[nodiscard]] constexpr std::vector<Move> generate(const Position& position);

/** Generates a list of all legal moves for only the given piece type in the given position.

    Generating King moves will include castling. Generating pawn moves will include all
    pushes, double pushes, captures, promotions, and en passant captures.

    @tparam PruneIllegal If true (the default), this function will return only strictly
    legal moves. If false, all pseudo-legal moves will be returned; that is, moves
    that obey the piece's movement mechanics, but may leave the side to move's king
    in check.

    @ingroup moves
    @see generate()
 */
template <bool PruneIllegal = true>
[[nodiscard]] constexpr std::vector<Move> generate_for(const Position& position, PieceType piece);

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

    namespace rank_masks = board::masks::ranks;
    namespace shifts     = board::shifts;

    static constexpr auto promotionMask = rank_masks::ONE | rank_masks::EIGHT;

    static constexpr std::array possiblePromotedTypes {
        PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen
    };

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr Rank prev_pawn_rank(const Rank rank) noexcept
    {
        if constexpr (Side == Color::White) {
            assert(rank != Rank::One);
            return static_cast<Rank>(std::to_underlying(rank) - 1uz);
        } else {
            assert(rank != Rank::Eight);
            return static_cast<Rank>(std::to_underlying(rank) + 1uz);
        }
    }

    template <Color Side>
    constexpr void add_pawn_pushes(
        const Bitboard ourPawns, const Bitboard emptySquares,
        std::output_iterator<Move> auto outputIt)
    {
        const auto allPushes = pseudo_legal::pawn_pushes<Side>(ourPawns, emptySquares);

        // non-promoting pushes
        {
            const auto pushes = allPushes & promotionMask.inverse();

            for (const auto target : pushes.squares())
                *outputIt = Move {
                    .from = {
                        .file = target.file,
                        .rank = prev_pawn_rank<Side>(target.rank) },
                    .to    = target,
                    .piece = PieceType::Pawn
                };
        }

        const auto promotions = allPushes & promotionMask;

        for (const auto target : promotions.squares())
            for (const auto promotedType : possiblePromotedTypes)
                *outputIt = Move {
                    .from = {
                        .file = target.file,
                        .rank = prev_pawn_rank<Side>(target.rank) },
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };
    }

    template <Color Side>
    constexpr void add_pawn_double_pushes(
        const Bitboard ourPawns, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        static constexpr auto pawnStartingRank = Side == Color::White ? Rank::Two : Rank::Seven;

        const auto pushes = pseudo_legal::pawn_double_pushes<Side>(ourPawns, allOccupied);

        for (const auto targetSquare : pushes.squares())
            *outputIt = Move {
                .from = {
                    .file = targetSquare.file,
                    .rank = pawnStartingRank },
                .to    = targetSquare,
                .piece = PieceType::Pawn
            };
    }

    template <Color Side>
    constexpr void add_pawn_captures(
        const Bitboard ourPawns, const Bitboard enemyPieces,
        std::output_iterator<Move> auto outputIt)
    {
        // We handle east & west captures separately to make set-wise operations easier.
        // This way, there is always a 1-1 relationship between a target square and a
        // starting square.

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
            *outputIt = Move {
                .from  = starting,
                .to    = target,
                .piece = PieceType::Pawn
            };
        }

        for (const auto [starting, target] : std::views::zip(canRegCaptureWest.squares(), westRegCaptures.squares())) {
            *outputIt = Move {
                .from  = starting,
                .to    = target,
                .piece = PieceType::Pawn
            };
        }

        for (const auto [starting, target] : std::views::zip(canCapturePromoteEast.squares(), eastPromotionCaptures.squares())) {
            for (const auto promotedType : possiblePromotedTypes) {
                *outputIt = Move {
                    .from         = starting,
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };
            }
        }

        for (const auto [starting, target] : std::views::zip(canCapturePromoteWest.squares(), westPromotionCaptures.squares())) {
            for (const auto promotedType : possiblePromotedTypes) {
                *outputIt = Move {
                    .from         = starting,
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };
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

        const Bitboard targetSquareBoard { targetSquare };

        const auto startSquares = shifts::pawn_inv_capture_east<Side>(targetSquareBoard)
                                | shifts::pawn_inv_capture_west<Side>(targetSquareBoard);

        const auto ourPawns = position.pieces_for<Side>().pawns;

        const auto eligiblePawns = ourPawns & startSquares;

        for (const auto square : eligiblePawns.squares()) {
            *outputIt = Move {
                .from  = square,
                .to    = targetSquare,
                .piece = PieceType::Pawn
            };
        }
    }

    template <Color Side>
    constexpr void add_all_pawn_moves(
        const Position& position, const Bitboard enemyPieces, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        const auto ourPawns = position.pieces_for<Side>().pawns;

        add_pawn_pushes<Side>(ourPawns, allOccupied.inverse(), outputIt);
        add_pawn_double_pushes<Side>(ourPawns, allOccupied, outputIt);
        add_pawn_captures<Side>(ourPawns, enemyPieces, outputIt);
        add_en_passant<Side>(position, outputIt);
    }

    constexpr void add_knight_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces,
        std::output_iterator<Move> auto outputIt)
    {
        // TODO: parallelize for all knights at once?
        for (const auto knightSquare : ourPieces.knights.squares()) {
            const auto knightMoves = pseudo_legal::knight(Bitboard { knightSquare }, friendlyPieces);

            for (const auto targetSquare : knightMoves.squares())
                *outputIt = Move {
                    .from  = knightSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Knight
                };
        }
    }

    constexpr void add_bishop_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces, const Bitboard emptySquares,
        std::output_iterator<Move> auto outputIt)
    {
        // TODO: parallelize for all bishops at once?
        for (const auto bishopSquare : ourPieces.bishops.squares()) {
            const auto bishopMoves = pseudo_legal::bishop(Bitboard { bishopSquare }, emptySquares, friendlyPieces);

            for (const auto targetSquare : bishopMoves.squares())
                *outputIt = Move {
                    .from  = bishopSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Bishop
                };
        }
    }

    constexpr void add_rook_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces, const Bitboard emptySquares,
        std::output_iterator<Move> auto outputIt)
    {
        // TODO: parallelize for all rooks at once?
        for (const auto rookSquare : ourPieces.rooks.squares()) {
            const auto rookMoves = pseudo_legal::rook(Bitboard { rookSquare }, emptySquares, friendlyPieces);

            for (const auto targetSquare : rookMoves.squares())
                *outputIt = Move {
                    .from  = rookSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Rook
                };
        }
    }

    constexpr void add_queen_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces, const Bitboard emptySquares,
        std::output_iterator<Move> auto outputIt)
    {
        // TODO: parallelize for all queens at once?
        for (const auto queenSquare : ourPieces.queens.squares()) {
            const auto queenMoves = pseudo_legal::queen(Bitboard { queenSquare }, emptySquares, friendlyPieces);

            for (const auto targetSquare : queenMoves.squares())
                *outputIt = Move {
                    .from  = queenSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Queen
                };
        }
    }

    constexpr void add_king_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces,
        std::output_iterator<Move> auto outputIt)
    {
        const auto kingMoves = pseudo_legal::king(ourPieces.king, friendlyPieces);

        const auto kingSquare = ourPieces.get_king_location();

        for (const auto targetSquare : kingMoves.squares())
            *outputIt = Move {
                .from  = kingSquare,
                .to    = targetSquare,
                .piece = PieceType::King
            };
    }

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
    template <Color Side, bool AllowAttacked>
    [[nodiscard, gnu::const]] consteval Bitboard queenside_castle_mask() noexcept
    {
        static constexpr auto rank = Side == Color::White ? Rank::One : Rank::Eight;

        Bitboard board;

        if constexpr (AllowAttacked) {
            board.set(Square { File::B, rank });
        }

        board.set(Square { File::C, rank });
        board.set(Square { File::D, rank });

        return board;
    }

    template <Color Side>
    constexpr void add_castling(
        const Position& position, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        static constexpr bool isWhite = Side == Color::White;

        const auto& rights = isWhite ? position.whiteCastlingRights : position.blackCastlingRights;

        if (! rights.either())
            return;

        static constexpr auto OppositeColor = isWhite ? Color::Black : Color::White;

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OppositeColor>();

        const auto allOurPieces = ourPieces.occupied();

        [[maybe_unused]] const auto& rooks = ourPieces.rooks;

        if (rights.kingside) {
            assert(rooks.test(Square { File::H, board::back_rank_for(position.sideToMove) }));

            static constexpr auto requiredSquares = kingside_castle_mask<Side>();

            const bool castlingBlocked = (requiredSquares & allOccupied).any()
                                      || squares_attacked<OppositeColor>(theirPieces, requiredSquares, allOurPieces);

            if (! castlingBlocked)
                *outputIt = castle_kingside(isWhite ? Color::White : Color::Black);
        }

        if (rights.queenside) {
            assert(rooks.test(Square { File::A, board::back_rank_for(position.sideToMove) }));

            static constexpr auto occupiedMask = queenside_castle_mask<Side, true>();
            static constexpr auto attackedMask = queenside_castle_mask<Side, false>();

            const bool castlingBlocked = (allOccupied & occupiedMask).any()
                                      || squares_attacked<OppositeColor>(theirPieces, attackedMask, allOurPieces);

            if (! castlingBlocked)
                *outputIt = castle_queenside(isWhite ? Color::White : Color::Black);
        }
    }

    template <Color Side>
    constexpr void generate_internal(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        static constexpr auto OppositeSide = Side == Color::White ? Color::Black : Color::White;

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OppositeSide>();

        const auto friendlyPieces = ourPieces.occupied();
        const auto enemyPieces    = theirPieces.occupied();

        const auto allOccupied  = friendlyPieces | enemyPieces;
        const auto emptySquares = allOccupied.inverse();

        add_all_pawn_moves<Side>(position, enemyPieces, allOccupied, outputIt);

        add_knight_moves(ourPieces, friendlyPieces, outputIt);

        add_bishop_moves(ourPieces, friendlyPieces, emptySquares, outputIt);

        add_rook_moves(ourPieces, friendlyPieces, emptySquares, outputIt);

        add_queen_moves(ourPieces, friendlyPieces, emptySquares, outputIt);

        add_king_moves(ourPieces, friendlyPieces, outputIt);

        add_castling<Side>(position, allOccupied, outputIt);
    }

    template <Color Side>
    constexpr void generate_for_internal(
        const Position& position, const PieceType piece,
        std::output_iterator<Move> auto outputIt)
    {
        static constexpr auto OppositeSide = Side == Color::White ? Color::Black : Color::White;

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OppositeSide>();

        const auto friendlyPieces = ourPieces.occupied();
        const auto enemyPieces    = theirPieces.occupied();

        const auto allOccupied  = friendlyPieces | enemyPieces;
        const auto emptySquares = allOccupied.inverse();

        switch (piece) {
            case PieceType::Pawn: {
                add_all_pawn_moves<Side>(position, enemyPieces, allOccupied, outputIt);
                return;
            }

            case PieceType::Knight: {
                add_knight_moves(ourPieces, friendlyPieces, outputIt);
                return;
            }

            case PieceType::Bishop: {
                add_bishop_moves(ourPieces, friendlyPieces, emptySquares, outputIt);
                return;
            }

            case PieceType::Rook: {
                add_rook_moves(ourPieces, friendlyPieces, emptySquares, outputIt);
                return;
            }

            case PieceType::Queen: {
                add_queen_moves(ourPieces, friendlyPieces, emptySquares, outputIt);
                return;
            }

            default: // King
                add_king_moves(ourPieces, friendlyPieces, outputIt);

                // castling is considered a King move
                add_castling<Side>(position, allOccupied, outputIt);
        }
    }

} // namespace detail

template <bool PruneIllegal>
constexpr std::vector<Move> generate(const Position& position)
{
    std::vector<Move> moves;

    // NB. the maximum number of possible legal moves in a position seems to be 218
    // reserve some extra memory to allow non-legal moves to be generated & pruned
    moves.reserve(300uz);

    if (position.sideToMove == Color::White)
        detail::generate_internal<Color::White>(position, std::back_inserter(moves));
    else
        detail::generate_internal<Color::Black>(position, std::back_inserter(moves));

    if constexpr (PruneIllegal) {
        std::erase_if(moves,
            [position](const Move& move) { return ! position.is_legal(move); });
    }

    return moves;
}

template <bool PruneIllegal>
constexpr std::vector<Move> generate_for(
    const Position& position, const PieceType piece)
{
    std::vector<Move> moves;

    if (position.sideToMove == Color::White)
        detail::generate_for_internal<Color::White>(position, piece, std::back_inserter(moves));
    else
        detail::generate_for_internal<Color::Black>(position, piece, std::back_inserter(moves));

    if constexpr (PruneIllegal) {
        std::erase_if(moves,
            [position](const Move& move) { return ! position.is_legal(move); });
    }

    return moves;
}

} // namespace chess::moves
