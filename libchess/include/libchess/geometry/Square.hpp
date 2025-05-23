/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#pragma once

#include <cstdint> // IWYU pragma: keep - for std::uint_fast64_t
#include <format>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>
#include <string_view>
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
 */
struct Square final {
    /** This square's file. */
    File file { File::A };

    /** This square's rank. */
    Rank rank { Rank::One };

    /** Calculates the rank and file corresponding to the given bitboard index.
        This function asserts if the passed ``index`` is greater than 63.

        @throws std::invalid_argument An exception will be thrown if the passed
        ``index`` is greater than 63.
     */
    [[nodiscard, gnu::const]] static constexpr Square from_index(BitboardIndex index);

    /** Creates a square from a string in algebraic notation, such as "A1", "H4", etc.

        This method recognizes either upper- or lower-case file letters. This method
        always throws if the input string is not 2 characters long.

        @throws std::invalid_argument An exception will be thrown if a square cannot be
        parsed correctly from the input string.
     */
    [[nodiscard, gnu::const]] static constexpr Square from_string(std::string_view text);

    /** Returns the bitboard bit index for this square.
        The returned index will be in the range ``[0,63]``.
     */
    [[nodiscard]] constexpr BitboardIndex index() const noexcept
    {
        return (std::to_underlying(rank) << static_cast<BitboardIndex>(3)) + std::to_underlying(file);
    }

    /** Returns true if two squares are equivalent. */
    [[nodiscard]] constexpr bool operator==(const Square& other) const noexcept
    {
        return rank == other.rank && file == other.file;
    }

    /// @name Area queries
    /// @{
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
    /// @}

    /** Returns true if this is a light square. */
    [[nodiscard]] constexpr bool is_light() const noexcept;

    /** Returns true if this is a dark square. */
    [[nodiscard]] constexpr bool is_dark() const noexcept { return ! is_light(); }
};

} // namespace chess

/** A formatter for Square objects.

    The formatter accepts the following format specifier arguments:
    @li ``i|I``: Tells the formatter to print the bitboard bit index for this square
    @li ``a|A``: Tells the formatter to print the algebraic notation of this square

    If no arguments are specified, the formatter prints the square's algebraic notation by default.

    @see chess::Square
 */
template <>
struct std::formatter<chess::Square> final {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        auto it = ctx.begin();

        if (it == ctx.end() || *it == '}')
            return it;

        do {
            switch (*it) {
                case 'i': [[fallthrough]];
                case 'I':
                    asIdx = true;
                    break;

                case 'a': [[fallthrough]];
                case 'A':
                    asIdx = false;
                    break;

                default:
                    throw std::format_error { "Unrecognized format argument" };
            }

            ++it;
        } while (! (it == ctx.end() || *it == '}'));

        ctx.advance_to(it);

        return it;
    }

    template <typename FormatContext>
    auto format(const chess::Square& square, FormatContext& ctx) const
    {
        if (asIdx)
            return std::format_to(ctx.out(), "{}", square.index());

        return std::format_to(
            ctx.out(), "{}{}",
            magic_enum::enum_name(square.file),
            std::to_underlying(square.rank) + static_cast<chess::BitboardIndex>(1));
    }

private:
    bool asIdx { false };
};

namespace chess {

constexpr bool Square::is_light() const noexcept
{
    auto is_even = [] [[nodiscard, gnu::const]] (const BitboardIndex index) {
        return (index & static_cast<BitboardIndex>(1)) == static_cast<BitboardIndex>(0);
    };

    const auto fileEven = is_even(std::to_underlying(file));
    const auto rankEven = is_even(std::to_underlying(rank));

    if (fileEven)
        return ! rankEven;

    return rankEven;
}

constexpr Square Square::from_index(const BitboardIndex index)
{
    if (std::cmp_greater(index, 63uz))
        throw std::invalid_argument {
            std::format("Cannot create Square from invalid bitboard index {}", index)
        };

    return {
        .file = static_cast<File>(index & static_cast<BitboardIndex>(7)),
        .rank = static_cast<Rank>(index >> static_cast<BitboardIndex>(3))
    };
}

constexpr Square Square::from_string(const std::string_view text)
{
    if (text.length() != 2uz)
        throw std::invalid_argument {
            std::format("Cannot parse Square from invalid input string: {}", text)
        };

    const auto file = [character = text.front()] {
        switch (character) {
            case 'a': [[fallthrough]];
            case 'A': return File::A;

            case 'b': [[fallthrough]];
            case 'B': return File::B;

            case 'c': [[fallthrough]];
            case 'C': return File::C;

            case 'd': [[fallthrough]];
            case 'D': return File::D;

            case 'e': [[fallthrough]];
            case 'E': return File::E;

            case 'f': [[fallthrough]];
            case 'F': return File::F;

            case 'g': [[fallthrough]];
            case 'G': return File::G;

            case 'h': [[fallthrough]];
            case 'H': return File::H;

            default:
                throw std::invalid_argument {
                    std::format("Cannot parse file from character: {}", character)
                };
        }
    }();

    const auto rank = [character = text.back()] {
        switch (character) {
            case '1': return Rank::One;
            case '2': return Rank::Two;
            case '3': return Rank::Three;
            case '4': return Rank::Four;
            case '5': return Rank::Five;
            case '6': return Rank::Six;
            case '7': return Rank::Seven;
            case '8': return Rank::Eight;

            default:
                throw std::invalid_argument {
                    std::format("Cannot parse rank from character: {}", character)
                };
        }
    }();

    return { file, rank };
}

} // namespace chess
