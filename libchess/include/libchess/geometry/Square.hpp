/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <utility>

namespace chess {

/** Unsigned integer type used for bitboard indices. */
using BitboardIndex = std::uint_fast64_t;

/** This enum describes the ranks of the chessboard.
    @see File
 */
enum class Rank : BitboardIndex {
    One,   ///< The first rank. This is the rank that white's king starts on.
    Two,   ///< The second rank. This is the rank that white's pawns start on.
    Three, ///< The third rank.
    Four,  ///< The fourth rank.
    Five,  ///< The fifth rank.
    Six,   ///< The sixth rank.
    Seven, ///< The seventh rank. This is the rank that black's pawns start on.
    Eight  ///< The back rank. This is the rank that black's king starts on.
};

/** This enum describes the files of the chess board.
    @see Rank
 */
enum class File : BitboardIndex {
    A, ///< The A file.
    B, ///< The B file.
    C, ///< The C file.
    D, ///< The D file. This is the file that the queens start on.
    E, ///< The E file. This is the file that the kings start on.
    F, ///< The F file.
    G, ///< The G file.
    H  ///< The H file.
};

/** This struct uniquely identifies a square on the chessboard via its rank and file,
    and provides mappings to and from bitboard indices.

    This class uses the "Least Significant File" mapping to calculate bitboard indices,
    as opposed to the "Least Significant Rank" mapping. This means that ranks are aligned
    to the eight consecutive bytes of a bitboard.

    This results in the following mapping of squares to bitboard indices:
    @verbatim
          A |  B |  C |  D |  E |  F |  G |  H |
    8  | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |
    7  | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 |
    6  | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |
    5  | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |
    4  | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
    3  | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |
    2  | 8  | 9  | 10 | 11 | 12 | 13 | 14 | 15 |
    1  | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  |
    @endverbatim

    @todo is_dark()/is_light()
    @todo std::formatter
    @todo from_string()
    @todo free functions to iterate ranks, files, diagonals
 */
struct Square final {
    /** This square's rank. */
    Rank rank { Rank::One };

    /** This square's file. */
    File file { File::A };

    /** Returns the bitboard bit index for this square.
        The returned index will be in the range ``[0,63]``.
     */
    [[nodiscard]] constexpr BitboardIndex index() const noexcept
    {
        return static_cast<BitboardIndex>(8) * static_cast<BitboardIndex>(rank) + static_cast<BitboardIndex>(file);
    }

    /** Calculates the rank and file corresponding to the given bitboard index.
        This function asserts if the passed ``index`` is greater than 63.
     */
    [[nodiscard]] static constexpr Square from_index(BitboardIndex index) noexcept
    {
        assert(std::cmp_less_equal(index, 63uz));

        return {
            .rank = static_cast<Rank>(index >> static_cast<BitboardIndex>(3)),
            .file = static_cast<File>(index & static_cast<BitboardIndex>(7))
        };
    }

    /** Returns true if this square is on the queenside (the A-D files). */
    [[nodiscard]] constexpr bool is_queenside() const noexcept
    {
        return std::cmp_less_equal(
            std::to_underlying(file), std::to_underlying(File::D));
    }

    /** Returns true if this square is on the kingside (the E-H files). */
    [[nodiscard]] constexpr bool is_kingside() const noexcept
    {
        return std::cmp_greater_equal(
            std::to_underlying(file), std::to_underlying(File::E));
    }

    /** Returns true if this square is within White's territory (the first through fourth ranks). */
    [[nodiscard]] constexpr bool is_white_territory() const noexcept
    {
        return std::cmp_less_equal(
            std::to_underlying(rank), std::to_underlying(Rank::Four));
    }

    /** Returns true if this square is within Black's territory (the fifth through eighth ranks). */
    [[nodiscard]] constexpr bool is_black_territory() const noexcept
    {
        return std::cmp_greater_equal(
            std::to_underlying(rank), std::to_underlying(Rank::Five));
    }
};

} // namespace chess
