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

/** @file
    This file provides move generation functions.
    @ingroup moves
 */

#pragma once

#include <algorithm>
#include <array>
#include <beman/inplace_vector/inplace_vector.hpp>
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
#include <optional>
#include <ranges>
#include <utility>
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

#ifndef DOXYGEN
namespace detail {

    using board::Bitboard;
    using board::Rank;
    using board::Square;
    using pieces::Color;

    using board::prev_pawn_rank;

    namespace rank_masks = board::masks::ranks;
    namespace shifts     = board::shifts;

    static constexpr auto PROMOTION_MASK     = rank_masks::ONE | rank_masks::EIGHT;
    static constexpr auto NOT_PROMOTION_MASK = PROMOTION_MASK.inverse();

    static constexpr std::array possiblePromotedTypes {
        PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen
    };

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto get_pawn_pushes(
        const Position& position, const Bitboard emptySquares)
    {
        using Pushes = beman::inplace_vector<Move, 32uz>;

        const auto allPushes = pseudo_legal::pawn_pushes<Side>(
            position.pieces_for<Side>().pawns,
            emptySquares);

        auto nonPromotingPushes = (allPushes & NOT_PROMOTION_MASK).squares()
                                | std::views::transform([](const Square target) {
                                      return Move {
                                          .from = {
                                              .file = target.file,
                                              .rank = prev_pawn_rank<Side>(target.rank) },
                                          .to    = target,
                                          .piece = PieceType::Pawn
                                      };
                                  })
                                | std::ranges::to<Pushes>();

        auto promotingPushes = possiblePromotedTypes
                             | std::views::transform([pushes = (allPushes & PROMOTION_MASK).squares()](
                                                         const PieceType promotedType) {
                                   return pushes
                                        | std::views::transform([promotedType](const Square target) {
                                              return Move {
                                                  .from = {
                                                      .file = target.file,
                                                      .rank = prev_pawn_rank<Side>(target.rank) },
                                                  .to           = target,
                                                  .piece        = PieceType::Pawn,
                                                  .promotedType = promotedType
                                              };
                                          });
                               })
                             | std::views::join
                             | std::ranges::to<Pushes>();

        std::array moveLists {
            std::move(nonPromotingPushes), std::move(promotingPushes)
        };

        return std::views::join(std::move(moveLists))
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto get_pawn_double_pushes(
        const Position& position, const Bitboard allOccupied)
    {
        static constexpr auto pawnStartingRank = Side == Color::White ? Rank::Two : Rank::Seven;

        const auto pushes = pseudo_legal::pawn_double_pushes<Side>(
            position.pieces_for<Side>().pawns,
            allOccupied);

        return pushes.squares()
             | std::views::transform([](const Square target) {
                   return Move {
                       .from = {
                           .file = target.file,
                           .rank = pawnStartingRank },
                       .to    = target,
                       .piece = PieceType::Pawn
                   };
               })
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    [[nodiscard, gnu::const]] constexpr auto get_pawn_captures_internal(
        const Bitboard           startingBoard,
        const Bitboard           targetBoard,
        std::optional<PieceType> promotedType = std::nullopt)
    {
        return std::views::zip(
                   startingBoard.squares(), targetBoard.squares())
             | std::views::transform([promotedType](const auto& tuple) {
                   const auto [starting, target] = tuple;

                   return Move {
                       .from         = starting,
                       .to           = target,
                       .piece        = PieceType::Pawn,
                       .promotedType = promotedType
                   };
               });
    }

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto get_pawn_captures(
        const Position& position)
    {
        // We handle east & west captures separately to make set-wise operations easier.
        // This way, there is always a 1-1 relationship between a target square and a
        // starting square.

        const auto ourPawns    = position.pieces_for<Side>().pawns;
        const auto enemyPieces = position.pieces_for<pieces::other_side<Side>()>().occupied;

        const auto eastCaptures = shifts::pawn_capture_east<Side>(ourPawns) & enemyPieces;
        const auto westCaptures = shifts::pawn_capture_west<Side>(ourPawns) & enemyPieces;

        const auto eastPromotionCaptures = eastCaptures & PROMOTION_MASK;
        const auto westPromotionCaptures = westCaptures & PROMOTION_MASK;

        const auto eastRegCaptures = eastCaptures & NOT_PROMOTION_MASK;
        const auto westRegCaptures = westCaptures & NOT_PROMOTION_MASK;

        // starting positions of pawns that can make captures
        const auto canCapturePromoteEast = shifts::pawn_inv_capture_east<Side>(eastPromotionCaptures);
        const auto canCapturePromoteWest = shifts::pawn_inv_capture_west<Side>(westPromotionCaptures);

        const auto canRegCaptureEast = shifts::pawn_inv_capture_east<Side>(eastRegCaptures);
        const auto canRegCaptureWest = shifts::pawn_inv_capture_west<Side>(westRegCaptures);

        // max number of possible pawn captures is 16 * 4 possible promoted types
        using PawnCaptures = beman::inplace_vector<Move, 64uz>;

        auto get_non_promoting_captures = [](const Bitboard startingBoard, const Bitboard targetBoard) {
            return get_pawn_captures_internal(startingBoard, targetBoard)
                 | std::ranges::to<PawnCaptures>();
        };

        auto get_promotion_captures = [](const Bitboard startingBoard, const Bitboard targetBoard) {
            return possiblePromotedTypes
                 | std::views::transform([startingBoard, targetBoard](const PieceType type) {
                       return get_pawn_captures_internal(startingBoard, targetBoard, type);
                   })
                 | std::views::join
                 | std::ranges::to<PawnCaptures>();
        };

        std::array moveLists {
            get_non_promoting_captures(canRegCaptureEast, eastRegCaptures),
            get_non_promoting_captures(canRegCaptureWest, westRegCaptures),
            get_promotion_captures(canCapturePromoteEast, eastPromotionCaptures),
            get_promotion_captures(canCapturePromoteWest, westPromotionCaptures)
        };

        return std::views::join(std::move(moveLists))
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto get_en_passant(
        const Position& position)
    {
        // at most 2 captures are possible at a time
        using EPMoves = beman::inplace_vector<Move, 2uz>;

        if (not position.enPassantTargetSquare.has_value()) {
            [[likely]];
            return EPMoves {};
        }

        const auto targetSquare = *position.enPassantTargetSquare;

        const auto targetSquareBoard = Bitboard::from_square(targetSquare);

        const auto startSquares = shifts::pawn_inv_capture_east<Side>(targetSquareBoard)
                                | shifts::pawn_inv_capture_west<Side>(targetSquareBoard);

        return (position.pieces_for<Side>().pawns & startSquares).squares()
             | std::views::transform([targetSquare](const Square square) {
                   return Move {
                       .from  = square,
                       .to    = targetSquare,
                       .piece = PieceType::Pawn
                   };
               })
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               })
             | std::ranges::to<EPMoves>();
    }

    template <Color Side, bool CapturesOnly>
    constexpr void add_all_pawn_moves(
        const Position& position, const Bitboard allOccupied,
        std::output_iterator<Move> auto outputIt)
    {
        if constexpr (not CapturesOnly) {
            std::ranges::copy(
                get_pawn_pushes<Side>(position, allOccupied.inverse()),
                outputIt);

            std::ranges::copy(
                get_pawn_double_pushes<Side>(position, allOccupied),
                outputIt);
        }

        std::ranges::copy(
            get_pawn_captures<Side>(position),
            outputIt);

        std::ranges::copy(
            get_en_passant<Side>(position),
            outputIt);
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] constexpr auto get_knight_moves(
        const Position& position)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        return ourPieces.knights.subboards()
             | std::views::transform([ourOccupied      = ourPieces.occupied,
                                         theirOccupied = position.pieces_for<pieces::other_side<Side>()>().occupied](
                                         const Bitboard knightPos) {
                   auto knightMoves = pseudo_legal::knight(knightPos, ourOccupied);

                   if constexpr (CapturesOnly) {
                       knightMoves &= theirOccupied;
                   }

                   return knightMoves.squares()
                        | std::views::transform([knightPos](const Square targetSquare) {
                              return Move {
                                  .from  = Square::from_index(knightPos.first()),
                                  .to    = targetSquare,
                                  .piece = PieceType::Knight
                              };
                          });
               })
             | std::views::join
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] auto get_bishop_moves(
        const Position& position,
        const Bitboard  occupiedSquares)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        return ourPieces.bishops.squares()
             | std::views::transform([occupiedSquares,
                                         ourOccupied   = ourPieces.occupied,
                                         theirOccupied = position.pieces_for<pieces::other_side<Side>()>().occupied](
                                         const Square bishopPos) {
                   auto bishopMoves = magics::bishop(bishopPos, occupiedSquares, ourOccupied);

                   if constexpr (CapturesOnly) {
                       bishopMoves &= theirOccupied;
                   }

                   return bishopMoves.squares()
                        | std::views::transform([bishopPos](const Square targetSquare) {
                              return Move {
                                  .from  = bishopPos,
                                  .to    = targetSquare,
                                  .piece = PieceType::Bishop
                              };
                          });
               })
             | std::views::join
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] auto get_rook_moves(
        const Position& position,
        const Bitboard  occupiedSquares)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        return ourPieces.rooks.squares()
             | std::views::transform([occupiedSquares,
                                         ourOccupied   = ourPieces.occupied,
                                         theirOccupied = position.pieces_for<pieces::other_side<Side>()>().occupied](
                                         const Square rookPos) {
                   auto rookMoves = magics::rook(rookPos, occupiedSquares, ourOccupied);

                   if constexpr (CapturesOnly) {
                       rookMoves &= theirOccupied;
                   }

                   return rookMoves.squares()
                        | std::views::transform([rookPos](const Square targetSquare) {
                              return Move {
                                  .from  = rookPos,
                                  .to    = targetSquare,
                                  .piece = PieceType::Rook
                              };
                          });
               })
             | std::views::join
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] auto get_queen_moves(
        const Position& position,
        const Bitboard  occupiedSquares)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        return ourPieces.queens.squares()
             | std::views::transform([occupiedSquares,
                                         ourOccupied   = ourPieces.occupied,
                                         theirOccupied = position.pieces_for<pieces::other_side<Side>()>().occupied](
                                         const Square queenPos) {
                   auto queenMoves = magics::queen(queenPos, occupiedSquares, ourOccupied);

                   if constexpr (CapturesOnly) {
                       queenMoves &= theirOccupied;
                   }

                   return queenMoves.squares()
                        | std::views::transform([queenPos](const Square targetSquare) {
                              return Move {
                                  .from  = queenPos,
                                  .to    = targetSquare,
                                  .piece = PieceType::Queen
                              };
                          });
               })
             | std::views::join
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] constexpr auto get_king_moves(
        const Position& position)
    {
        const auto& ourPieces = position.pieces_for<Side>();

        auto kingMoves = pseudo_legal::king(ourPieces.king, ourPieces.occupied);

        if constexpr (CapturesOnly) {
            kingMoves &= position.pieces_for<pieces::other_side<Side>()>().occupied;
        }

        return kingMoves.squares()
             | std::views::transform([kingSquare = ourPieces.get_king_location()](
                                         const Square targetSquare) {
                   return Move {
                       .from  = kingSquare,
                       .to    = targetSquare,
                       .piece = PieceType::King
                   };
               })
             | std::views::filter([&position](const Move& move) {
                   return position.is_legal(move);
               });
    }

    // the two functions below are masks containing the set of squares that
    // must not be attacked/occupied in order for castling to be allowed

    template <Color Side>
    [[nodiscard, gnu::const]] consteval Bitboard kingside_castle_mask() noexcept
    {
        static constexpr auto rank = Side == Color::White ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { .file = File::F, .rank = rank });
        board.set(Square { .file = File::G, .rank = rank });

        return board;
    }

    // NB. with queenside castling, the set of squares that must be free/not attacked differ,
    // since castling is possible if the B1/B8 squares are attacked, but not if they are occupied
    template <Color Side, bool Occupied>
    [[nodiscard, gnu::const]] consteval Bitboard queenside_castle_mask() noexcept
    {
        static constexpr auto rank = Side == Color::White ? Rank::One : Rank::Eight;

        Bitboard board;

        board.set(Square { .file = File::C, .rank = rank });
        board.set(Square { .file = File::D, .rank = rank });

        if constexpr (Occupied) {
            board.set(Square { .file = File::B, .rank = rank });
        }

        return board;
    }

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto get_castling(
        const Position& position, const Bitboard allOccupied)
    {
        beman::inplace_vector<Move, 2uz> moves;

        // castling out of check is not allowed
        if (position.is_check())
            return moves;

        const auto& rights = Side == Color::White ? position.whiteCastlingRights : position.blackCastlingRights;

        static constexpr auto OppositeColor = pieces::other_side<Side>();

        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<OppositeColor>();

        if (rights.kingside) {
            assert(ourPieces.rooks.test(Square { File::H, board::back_rank_for(position.sideToMove) }));

            static constexpr auto requiredSquares = kingside_castle_mask<Side>();

            const bool castlingBlocked = (requiredSquares & allOccupied).any()
                                      or squares_attacked<OppositeColor>(theirPieces, requiredSquares, ourPieces.occupied);

            if (not castlingBlocked) {
                const auto move = castle_kingside(Side);

                if (position.is_legal(move))
                    moves.emplace_back(move);
            }
        }

        if (rights.queenside) {
            assert(ourPieces.rooks.test(Square { File::A, board::back_rank_for(position.sideToMove) }));

            static constexpr auto occupiedMask = queenside_castle_mask<Side, true>();
            static constexpr auto attackedMask = queenside_castle_mask<Side, false>();

            const bool castlingBlocked = (allOccupied & occupiedMask).any()
                                      or squares_attacked<OppositeColor>(theirPieces, attackedMask, ourPieces.occupied);

            if (not castlingBlocked) {
                const auto move = castle_queenside(Side);

                if (position.is_legal(move))
                    moves.emplace_back(move);
            }
        }

        return moves;
    }

    template <Color Side, bool CapturesOnly>
    void generate_internal(
        const Position&                 position,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<pieces::other_side<Side>()>();

        const auto allOccupied  = ourPieces.occupied | theirPieces.occupied;
        const auto emptySquares = allOccupied.inverse();

        add_all_pawn_moves<Side, CapturesOnly>(position, allOccupied, outputIt);

        std::ranges::copy(
            get_knight_moves<Side, CapturesOnly>(position),
            outputIt);

        std::ranges::copy(
            get_bishop_moves<Side, CapturesOnly>(position, allOccupied),
            outputIt);

        std::ranges::copy(
            get_rook_moves<Side, CapturesOnly>(position, allOccupied),
            outputIt);

        std::ranges::copy(
            get_queen_moves<Side, CapturesOnly>(position, allOccupied),
            outputIt);

        std::ranges::copy(
            get_king_moves<Side, CapturesOnly>(position),
            outputIt);

        if constexpr (not CapturesOnly) {
            std::ranges::copy(
                get_castling<Side>(position, allOccupied),
                outputIt);
        }
    }

    template <Color Side, bool CapturesOnly>
    void generate_for_internal(
        const Position& position, const PieceType piece,
        std::output_iterator<Move> auto outputIt)
    {
        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<pieces::other_side<Side>()>();

        const auto allOccupied  = ourPieces.occupied | theirPieces.occupied;
        const auto emptySquares = allOccupied.inverse();

        switch (piece) {
            case PieceType::Pawn: {
                add_all_pawn_moves<Side, CapturesOnly>(position, allOccupied, outputIt);
                return;
            }

            case PieceType::Knight: {
                std::ranges::copy(
                    get_knight_moves<Side, CapturesOnly>(position),
                    outputIt);
                return;
            }

            case PieceType::Bishop: {
                std::ranges::copy(
                    get_bishop_moves<Side, CapturesOnly>(position, allOccupied),
                    outputIt);
                return;
            }

            case PieceType::Rook: {
                std::ranges::copy(
                    get_rook_moves<Side, CapturesOnly>(position, allOccupied),
                    outputIt);
                return;
            }

            case PieceType::Queen: {
                std::ranges::copy(
                    get_queen_moves<Side, CapturesOnly>(position, allOccupied),
                    outputIt);
                return;
            }

            default: // King
                std::ranges::copy(
                    get_king_moves<Side, CapturesOnly>(position),
                    outputIt);

                if constexpr (not CapturesOnly) {
                    // castling is considered a King move
                    std::ranges::copy(
                        get_castling<Side>(position, allOccupied),
                        outputIt);
                }
        }
    }

    template <Color Side>
    [[nodiscard]] bool any_legal_moves_internal(const Position& position)
    {
        // optimize this function by avoiding dynamic allocation, since the number
        // of moves we're generating at a time should be relatively small
        beman::inplace_vector<Move, 100uz> moves;

        // as an optimization, check for king moves first, because in a double check,
        // a king move would be the only valid response
        for (const auto piece : { PieceType::King, PieceType::Pawn, PieceType::Knight, PieceType::Queen, PieceType::Rook, PieceType::Bishop }) {
            generate_for_internal<Side, false>(position, piece, std::back_inserter(moves));

            if (not moves.empty())
                return true;

            moves.clear();
        }

        return false;
    }

} // namespace detail
#endif // DOXYGEN

template <bool CapturesOnly>
void generate(
    const Position&                 position,
    std::output_iterator<Move> auto outputIt)
{
    if (position.is_white_to_move())
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
    if (position.is_white_to_move())
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
    if (position.is_white_to_move())
        return detail::any_legal_moves_internal<Color::White>(position);

    return detail::any_legal_moves_internal<Color::Black>(position);
}

} // namespace chess::moves
