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

/** Generates a list of all legal moves for the side to move in the given position.
    If the side to move is in checkmate or stalemate, this returns an empty list.

    @ingroup moves
 */
[[nodiscard]] constexpr std::vector<Move> generate_legal_moves(const Position& position);

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

    using PieceType = pieces::Type;

    namespace rank_masks = board::masks::ranks;

    static constexpr auto promotionMask = rank_masks::one() | rank_masks::eight();

    static constexpr std::array possiblePromotedTypes {
        PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen
    };

    [[nodiscard, gnu::const]] constexpr Rank prev_pawn_rank(
        const Rank rank, const bool isWhite) noexcept
    {
        if (isWhite) {
            assert(rank != Rank::One);
            return static_cast<Rank>(std::to_underlying(rank) - 1uz);
        }

        assert(rank != Rank::Eight);
        return static_cast<Rank>(std::to_underlying(rank) + 1uz);
    }

    constexpr void add_pawn_pushes(
        const Pieces& ourPieces, const bool isWhite, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        const auto allPushes = isWhite
                                 ? pseudo_legal::pawn_pushes<Color::White>(ourPieces.pawns, allOccupied)
                                 : pseudo_legal::pawn_pushes<Color::Black>(ourPieces.pawns, allOccupied);

        // non-promoting pushes
        {
            const auto pushes = allPushes & promotionMask.inverse();

            for (const auto target : pushes.squares())
                *outputIt = Move {
                    .from = {
                        .file = target.file,
                        .rank = prev_pawn_rank(target.rank, isWhite) },
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
                        .rank = prev_pawn_rank(target.rank, isWhite) },
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };
    }

    constexpr void add_pawn_double_pushes(
        const Pieces& ourPieces, const bool isWhite, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        const auto pawnStartingRank = isWhite ? Rank::Two : Rank::Seven;

        const auto pushes = isWhite
                              ? pseudo_legal::pawn_double_pushes<Color::White>(ourPieces.pawns, allOccupied)
                              : pseudo_legal::pawn_double_pushes<Color::Black>(ourPieces.pawns, allOccupied);

        for (const auto targetSquare : pushes.squares())
            *outputIt = Move {
                .from = {
                    .file = targetSquare.file,
                    .rank = pawnStartingRank },
                .to    = targetSquare,
                .piece = PieceType::Pawn
            };
    }

    constexpr void add_pawn_captures(
        const Pieces& ourPieces, const bool isWhite, const Bitboard enemyPieces,
        std::output_iterator<Move> auto outputIt)
    {
        // TODO: do these set-wise?
        for (const auto starting : ourPieces.pawns.squares()) {
            const Bitboard startingBoard { starting };

            const auto capture = isWhite
                                   ? pseudo_legal::pawn_captures<Color::White>(startingBoard, enemyPieces)
                                   : pseudo_legal::pawn_captures<Color::Black>(startingBoard, enemyPieces);

            if (capture.none())
                continue;

            const bool isPromotion = (capture & promotionMask).any();

            const auto target = Square::from_index(capture.first());

            if (! isPromotion) {
                [[likely]];

                *outputIt = Move {
                    .from  = starting,
                    .to    = target,
                    .piece = PieceType::Pawn
                };

                continue;
            }

            for (const auto promotedType : possiblePromotedTypes)
                *outputIt = Move {
                    .from         = starting,
                    .to           = target,
                    .piece        = PieceType::Pawn,
                    .promotedType = promotedType
                };
        }
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
        const Pieces& ourPieces, const Bitboard friendlyPieces, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        for (const auto bishopSquare : ourPieces.bishops.squares()) {
            const auto bishopMoves = pseudo_legal::bishop(bishopSquare, allOccupied, friendlyPieces);

            for (const auto targetSquare : bishopMoves.squares())
                *outputIt = Move {
                    .from  = bishopSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Bishop
                };
        }
    }

    constexpr void add_rook_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        for (const auto rookSquare : ourPieces.rooks.squares()) {
            const auto rookMoves = pseudo_legal::rook(rookSquare, allOccupied, friendlyPieces);

            for (const auto targetSquare : rookMoves.squares())
                *outputIt = Move {
                    .from  = rookSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Rook
                };
        }
    }

    constexpr void add_queen_moves(
        const Pieces& ourPieces, const Bitboard friendlyPieces, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        for (const auto queenSquare : ourPieces.queens.squares()) {
            const auto queenMoves = pseudo_legal::queen(queenSquare, allOccupied, friendlyPieces);

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

    constexpr void add_en_passant(
        const Square& targetSquare, const bool isWhite, const Bitboard ourPawns,
        std::output_iterator<Move> auto outputIt)
    {
        const Bitboard targetSquareBoard { targetSquare };

        // TODO: do this set-wise?
        for (const auto pawnSquare : ourPawns.squares()) {
            const Bitboard pawnBoard { pawnSquare };

            const auto captures = isWhite
                                    ? patterns::pawn_attacks<Color::White>(pawnBoard)
                                    : patterns::pawn_attacks<Color::Black>(pawnBoard);

            if ((captures & targetSquareBoard).any())
                *outputIt = Move {
                    .from  = pawnSquare,
                    .to    = targetSquare,
                    .piece = PieceType::Pawn
                };
        }
    }

    [[nodiscard, gnu::const]] constexpr Bitboard kingside_castle_mask(const bool isWhite) noexcept
    {
        const auto rank = isWhite ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { File::F, rank });
        board.set(Square { File::G, rank });

        return board;
    }

    // NB. with queenside castling, the set of squares that must be free/not attacked differ,
    // since castling is possible if the B1/B8 squares are attacked, but not if they are occupied
    [[nodiscard, gnu::const]] constexpr Bitboard queenside_castle_mask_occupied(const bool isWhite) noexcept
    {
        const auto rank = isWhite ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { File::B, rank });
        board.set(Square { File::C, rank });
        board.set(Square { File::D, rank });

        return board;
    }

    [[nodiscard, gnu::const]] constexpr Bitboard queenside_castle_mask_attacked(const bool isWhite) noexcept
    {
        const auto rank = isWhite ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { File::C, rank });
        board.set(Square { File::D, rank });

        return board;
    }

    constexpr void add_castling(
        const Position& position, const bool isWhite, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& rights = isWhite ? position.whiteCastlingRights : position.blackCastlingRights;

        if (! rights.either())
            return;

        [[maybe_unused]] const auto& rooks = isWhite ? position.whitePieces.rooks : position.blackPieces.rooks;

        const auto opponentAttacks = isWhite
                                       ? board::attacked_squares<Color::Black>(position.blackPieces, position.whitePieces.occupied())
                                       : board::attacked_squares<Color::White>(position.whitePieces, position.blackPieces.occupied());

        if (rights.kingside) {
            assert(rooks.test(Square { File::H, board::back_rank_for(position.sideToMove) }));

            const auto requiredSquares = kingside_castle_mask(isWhite);

            const bool castlingBlocked = (requiredSquares & allOccupied).any()
                                      || (requiredSquares & opponentAttacks).any();

            if (! castlingBlocked)
                *outputIt = castle_kingside(isWhite ? Color::White : Color::Black);
        }

        if (rights.queenside) {
            assert(rooks.test(Square { File::A, board::back_rank_for(position.sideToMove) }));

            const bool castlingBlocked = (allOccupied & queenside_castle_mask_occupied(isWhite)).any()
                                      || (opponentAttacks & queenside_castle_mask_attacked(isWhite)).any();

            if (! castlingBlocked)
                *outputIt = castle_queenside(isWhite ? Color::White : Color::Black);
        }
    }

} // namespace detail

constexpr std::vector<Move> generate_legal_moves(const Position& position)
{
    using pieces::Color;

    const bool isWhite = position.sideToMove == Color::White;

    const auto& ourPieces   = isWhite ? position.whitePieces : position.blackPieces;
    const auto& theirPieces = isWhite ? position.blackPieces : position.whitePieces;

    const auto friendlyPieces = ourPieces.occupied();
    const auto enemyPieces    = theirPieces.occupied();

    const auto allOccupied = friendlyPieces | enemyPieces;

    std::vector<Move> moves;

    // NB. the maximum number of possible legal moves in a position seems to be 218
    // reserve some extra memory to allow non-legal moves to be generated & pruned
    moves.reserve(300uz);

    detail::add_pawn_pushes(
        ourPieces, isWhite, allOccupied, std::back_inserter(moves));

    detail::add_pawn_double_pushes(
        ourPieces, isWhite, allOccupied, std::back_inserter(moves));

    detail::add_pawn_captures(
        ourPieces, isWhite, enemyPieces, std::back_inserter(moves));

    detail::add_knight_moves(
        ourPieces, friendlyPieces, std::back_inserter(moves));

    detail::add_bishop_moves(
        ourPieces, friendlyPieces, allOccupied, std::back_inserter(moves));

    detail::add_rook_moves(
        ourPieces, friendlyPieces, allOccupied, std::back_inserter(moves));

    detail::add_queen_moves(
        ourPieces, friendlyPieces, allOccupied, std::back_inserter(moves));

    detail::add_king_moves(
        ourPieces, friendlyPieces, std::back_inserter(moves));

    detail::add_castling(
        position, isWhite, allOccupied, std::back_inserter(moves));

    if (position.enPassantTargetSquare.has_value()) {
        detail::add_en_passant(
            *position.enPassantTargetSquare, isWhite, ourPieces.pawns, std::back_inserter(moves));
    }

    std::erase_if(moves,
        [position](const Move& move) { return ! position.is_legal(move); });

    return moves;
}

} // namespace chess::moves
