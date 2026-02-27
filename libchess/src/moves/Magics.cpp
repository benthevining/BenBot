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

#include "moves/MagicData.hpp"
#include <array>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <ranges>
#include <utility>
#include <vector>

namespace chess::moves::magics {

namespace {

    using board::BitboardIndex;

    // Occupancy masks
    // The next 2 functions generate masks for the "relevant occupancy bits" for a given starting square.
    // This is basically the piece type's movement pattern, except excluding the final square in each
    // ray direction, since for blockers it doesn't matter if there's a piece there or not.

    namespace masks = board::masks;

    using MaskArray = std::array<Bitboard, board::NUM_SQUARES>;

    [[nodiscard]] consteval auto calculate_bishop_masks() -> MaskArray
    {
        MaskArray result {};

        std::ranges::transform(
            masks::ALL.subboards(),
            result.data(),
            [](const Bitboard square) {
                return patterns::bishop(square) & masks::PERIMETER.inverse();
            });

        return result;
    }

    [[nodiscard]] consteval auto calculate_rook_masks() -> MaskArray
    {
        using board::File;
        using board::Rank;

        MaskArray result {};

        for (const auto square : masks::ALL.squares()) {
            auto& value = result.at(square.index());

            const auto file = static_cast<int>(std::to_underlying(square.file));
            const auto rank = static_cast<int>(std::to_underlying(square.rank));

            for (auto r = rank + 1; r <= 6; ++r) {
                value |= Bitboard::from_square(Square {
                    .file = square.file,
                    .rank = static_cast<Rank>(r) });
            }

            for (auto r = rank - 1; r >= 1; --r) {
                value |= Bitboard::from_square(Square {
                    .file = square.file,
                    .rank = static_cast<Rank>(r) });
            }

            for (auto f = file + 1; f <= 6; ++f) {
                value |= Bitboard::from_square(Square {
                    .file = static_cast<File>(f),
                    .rank = square.rank });
            }

            for (auto f = file - 1; f >= 1; --f) {
                value |= Bitboard::from_square(Square {
                    .file = static_cast<File>(f),
                    .rank = square.rank });
            }
        }

        return result;
    }

    inline constexpr auto BISHOP_MASKS = calculate_bishop_masks();
    inline constexpr auto ROOK_MASKS   = calculate_rook_masks();

    // the next two functions calculate indices within the MagicMoves
    // array for the given piece type, square, and occupied squares

    [[nodiscard, gnu::const]] constexpr auto calc_bishop_index(
        const size_t squareIdx, const Bitboard occupied) -> size_t
    {
        const auto [mul, offset] = data::BISHOP_MAGICS.at(squareIdx);

        const auto mask = BISHOP_MASKS.at(squareIdx);

        return static_cast<size_t>(offset)
             + (((occupied & mask).to_int() * mul) >> data::BISHOP_SHIFT);
    }

    [[nodiscard, gnu::const]] constexpr auto calc_rook_index(
        const size_t squareIdx, const Bitboard occupied) -> size_t
    {
        const auto [mul, offset] = data::ROOK_MAGICS.at(squareIdx);

        const auto mask = ROOK_MASKS.at(squareIdx);

        return static_cast<size_t>(offset)
             + (((occupied & mask).to_int() * mul) >> data::ROOK_SHIFT);
    }

    // returns the next permutation of the given set
    // used to generate all permutations of possible blockers
    [[nodiscard, gnu::const]] constexpr auto permute(
        const Bitboard set, const Bitboard subset) noexcept -> Bitboard
    {
        return Bitboard { subset.to_int() - set.to_int() } & set;
    }

    // NB. this isn't std::array because we encountered stack overflows when building with MSVC
    using MagicMoves = std::vector<Bitboard>;

    [[nodiscard]] constexpr auto generate_magic_moves() -> MagicMoves
    {
        MagicMoves result;

        result.resize(88772uz);

        for (auto i = 0uz; i < board::NUM_SQUARES; ++i) {
            const auto square = Square::from_index(static_cast<BitboardIndex>(i));

            // Bishops
            {
                Bitboard occupied;

                do {
                    auto& value = result.at(calc_bishop_index(i, occupied));

                    value = pseudo_legal::bishop(
                        Bitboard::from_square(square),
                        occupied.inverse(), {});

                    occupied = permute(BISHOP_MASKS.at(i), occupied);
                } while (occupied.any());
            }

            // Rooks
            {
                Bitboard occupied;

                do {
                    auto& value = result.at(calc_rook_index(i, occupied));

                    value = pseudo_legal::rook(
                        Bitboard::from_square(square),
                        occupied.inverse(), {});

                    occupied = permute(ROOK_MASKS.at(i), occupied);
                } while (occupied.any());
            }
        }

        result.shrink_to_fit();

        return result;
    }

    // NB. this array can't be constexpr because we currently
    // hit every compiler's constexpr step limit
    const MagicMoves MAGIC_MOVES { generate_magic_moves() };

} // namespace

auto bishop(
    const Square& bishopPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
    -> Bitboard
{
    const auto moves = MAGIC_MOVES.at(
        calc_bishop_index(bishopPos.index(), occupiedSquares));

    return moves & friendlyPieces.inverse();
}

auto rook(
    const Square& rookPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
    -> Bitboard
{
    const auto moves = MAGIC_MOVES.at(
        calc_rook_index(rookPos.index(), occupiedSquares));

    return moves & friendlyPieces.inverse();
}

auto queen(
    const Square& queenPos, const Bitboard occupiedSquares, const Bitboard friendlyPieces)
    -> Bitboard
{
    const auto squareIdx = queenPos.index();

    const auto bishopMoves = MAGIC_MOVES.at(
        calc_bishop_index(squareIdx, occupiedSquares));

    const auto rookMoves = MAGIC_MOVES.at(
        calc_rook_index(squareIdx, occupiedSquares));

    return (bishopMoves | rookMoves) & friendlyPieces.inverse();
}

} // namespace chess::moves::magics
