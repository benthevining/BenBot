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
#include <beman/inplace_vector/inplace_vector.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Shifts.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Attacks.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/moves/MoveRange.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <range/v3/view/concat.hpp>
#include <ranges>
#include <utility>

namespace chess::moves {

namespace {

    using board::Bitboard;
    using board::Rank;
    using board::Square;
    using pieces::Color;

    using board::prev_pawn_rank;

    namespace rank_masks = board::masks::ranks;
    namespace shifts     = board::shifts;

    constexpr auto PROMOTION_MASK     = rank_masks::ONE | rank_masks::EIGHT;
    constexpr auto NOT_PROMOTION_MASK = PROMOTION_MASK.inverse();

    constexpr std::array possiblePromotedTypes {
        PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen
    };

    template <Color Side>
    [[nodiscard, gnu::const]] auto get_pawn_pushes(
        const Position& position, const Bitboard emptySquares)
    {
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
                                  });

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
                             | std::views::join;

        return ranges::concat_view {
            std::move(nonPromotingPushes),
            std::move(promotingPushes)
        };
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
    [[nodiscard, gnu::const]] auto get_pawn_captures(
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

        auto get_promotion_captures = [](const Bitboard startingBoard, const Bitboard targetBoard) {
            return possiblePromotedTypes
                 | std::views::transform([startingBoard, targetBoard](const PieceType type) {
                       return get_pawn_captures_internal(startingBoard, targetBoard, type);
                   })
                 | std::views::join;
        };

        return ranges::concat_view {
            get_pawn_captures_internal(canRegCaptureEast, eastRegCaptures),
            get_pawn_captures_internal(canRegCaptureWest, westRegCaptures),
            get_promotion_captures(canCapturePromoteEast, eastPromotionCaptures),
            get_promotion_captures(canCapturePromoteWest, westPromotionCaptures)
        };
    }

    template <Color Side>
    [[nodiscard, gnu::const]] constexpr auto get_en_passant(const Position& position)
    {
        const auto targetSquareBoard = position.enPassantTargetSquare
                                           .transform([](const Square targetSquare) { return Bitboard::from_square(targetSquare); })
                                           .value_or(Bitboard {});

        const auto startSquares = shifts::pawn_inv_capture_east<Side>(targetSquareBoard)
                                | shifts::pawn_inv_capture_west<Side>(targetSquareBoard);

        return (position.pieces_for<Side>().pawns & startSquares).squares()
             | std::views::transform([targetSquare = Square::from_index(targetSquareBoard.first())](
                                         const Square square) {
                   return Move {
                       .from  = square,
                       .to    = targetSquare,
                       .piece = PieceType::Pawn
                   };
               });
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] auto get_all_pawn_moves(
        const Position& position, const Bitboard allOccupied)
    {
        auto captures  = get_pawn_captures<Side>(position);
        auto enPassant = get_en_passant<Side>(position);

        if constexpr (CapturesOnly) {
            return ranges::concat_view {
                std::move(captures),
                std::move(enPassant)
            };
        } else {
            return ranges::concat_view {
                std::move(captures),
                std::move(enPassant),
                get_pawn_pushes<Side>(position, allOccupied.inverse()),
                get_pawn_double_pushes<Side>(position, allOccupied)
            };
        }
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
             | std::views::join;
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
             | std::views::join;
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
             | std::views::join;
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
             | std::views::join;
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] constexpr auto get_king_moves(const Position& position)
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

            if (not castlingBlocked)
                moves.emplace_back(castle_kingside(Side));
        }

        if (rights.queenside) {
            assert(ourPieces.rooks.test(Square { File::A, board::back_rank_for(position.sideToMove) }));

            static constexpr auto occupiedMask = queenside_castle_mask<Side, true>();
            static constexpr auto attackedMask = queenside_castle_mask<Side, false>();

            const bool castlingBlocked = (allOccupied & occupiedMask).any()
                                      or squares_attacked<OppositeColor>(theirPieces, attackedMask, ourPieces.occupied);

            if (not castlingBlocked)
                moves.emplace_back(castle_queenside(Side));
        }

        return moves;
    }

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] MoveRange get_all(const Position& position)
    {
        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<pieces::other_side<Side>()>();

        const auto allOccupied  = ourPieces.occupied | theirPieces.occupied;
        const auto emptySquares = allOccupied.inverse();

        ranges::concat_view moves {
            get_all_pawn_moves<Side, CapturesOnly>(position, allOccupied),
            get_knight_moves<Side, CapturesOnly>(position),
            get_bishop_moves<Side, CapturesOnly>(position, allOccupied),
            get_rook_moves<Side, CapturesOnly>(position, allOccupied),
            get_queen_moves<Side, CapturesOnly>(position, allOccupied),
            get_king_moves<Side, CapturesOnly>(position)
        };

        if constexpr (CapturesOnly) {
            return MoveRange { std::move(moves)
                               | std::views::filter([position](const Move& move) {
                                     return position.is_legal(move);
                                 }) };
        } else {
            return MoveRange { ranges::concat_view {
                                   std::move(moves),
                                   get_castling<Side>(position, allOccupied) }
                               | std::views::filter([position](const Move& move) {
                                     return position.is_legal(move);
                                 }) };
        }
    }

} // namespace

template <bool CapturesOnly>
MoveRange generate(const Position& position)
{
    if (position.is_white_to_move())
        return get_all<Color::White, CapturesOnly>(position);

    return get_all<Color::Black, CapturesOnly>(position);
}

template MoveRange generate<true>(const Position&);
template MoveRange generate<false>(const Position&);

namespace {

    template <Color Side, bool CapturesOnly>
    [[nodiscard, gnu::const]] MoveRange generate_for_internal(
        const Position& position, const PieceType piece)
    {
        const auto& ourPieces   = position.pieces_for<Side>();
        const auto& theirPieces = position.pieces_for<pieces::other_side<Side>()>();

        const auto allOccupied  = ourPieces.occupied | theirPieces.occupied;
        const auto emptySquares = allOccupied.inverse();

        switch (piece) {
            case PieceType::Pawn: {
                return MoveRange {
                    get_all_pawn_moves<Side, CapturesOnly>(position, allOccupied)
                    | std::views::filter([&position](const Move& move) {
                          return position.is_legal(move);
                      })
                };
            }

            case PieceType::Knight: {
                return MoveRange {
                    get_knight_moves<Side, CapturesOnly>(position)
                    | std::views::filter([&position](const Move& move) {
                          return position.is_legal(move);
                      })
                };
            }

            case PieceType::Bishop: {
                return MoveRange {
                    get_bishop_moves<Side, CapturesOnly>(position, allOccupied)
                    | std::views::filter([&position](const Move& move) {
                          return position.is_legal(move);
                      })
                };
            }

            case PieceType::Rook: {
                return MoveRange {
                    get_rook_moves<Side, CapturesOnly>(position, allOccupied)
                    | std::views::filter([&position](const Move& move) {
                          return position.is_legal(move);
                      })
                };
            }

            case PieceType::Queen: {
                return MoveRange {
                    get_queen_moves<Side, CapturesOnly>(position, allOccupied)
                    | std::views::filter([&position](const Move& move) {
                          return position.is_legal(move);
                      })
                };
            }

            default: // King
                auto kingMoves = get_king_moves<Side, CapturesOnly>(position);

                if constexpr (CapturesOnly) {
                    return MoveRange {
                        std::move(kingMoves)
                        | std::views::filter([&position](const Move& move) {
                              return position.is_legal(move);
                          })
                    };
                } else {
                    return MoveRange {
                        ranges::concat_view {
                            std::move(kingMoves),
                            get_castling<Side>(position, allOccupied) }
                        | std::views::filter([&position](const Move& move) {
                              return position.is_legal(move);
                          })
                    };
                }
        }
    }

} // namespace

template <bool CapturesOnly>
MoveRange generate_for(
    const Position& position, const PieceType piece)
{
    if (position.is_white_to_move())
        return generate_for_internal<Color::White, CapturesOnly>(position, piece);

    return generate_for_internal<Color::Black, CapturesOnly>(position, piece);
}

template MoveRange generate_for<true>(const Position&, PieceType);
template MoveRange generate_for<false>(const Position&, PieceType);

namespace {

    template <Color Side>
    [[nodiscard]] bool any_legal_moves_internal(const Position& position)
    {
        return std::ranges::any_of(
            magic_enum::enum_values<PieceType>(),
            [&position](const PieceType type) {
                return not generate_for_internal<Side, false>(position, type).empty();
            });
    }

} // namespace

bool any_legal_moves(const Position& position)
{
    if (position.is_white_to_move())
        return any_legal_moves_internal<Color::White>(position);

    return any_legal_moves_internal<Color::Black>(position);
}

} // namespace chess::moves
