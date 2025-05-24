/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @defgroup board Board representation
    Classes related to the engine's internal board representation.
    libchess uses bitboards with "Least Significant File" encoding.
 */

/** @file
    This file defines the Square class and related enums.

    @ingroup board
 */

#pragma once

#include <cctype>
#include <cstdint> // IWYU pragma: keep - for std::uint_fast64_t
#include <format>
#include <libchess/util/Math.hpp>
#include <magic_enum/magic_enum.hpp>
#include <stdexcept>
#include <string_view>
#include <utility>

/** This namespace contains classes related to the engine's internal
    board representation.

    @ingroup board
 */
namespace chess::board {

/** Unsigned integer type used for bitboard indices.
    Valid bitboard indices are in the range ``[0, 63]``.

    @ingroup board
    @todo Make this an enum?
 */
using BitboardIndex = std::uint_fast8_t;

/** This enum describes the ranks of the chessboard.

    @see File
    @ingroup board
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
    @ingroup board
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

    Rank |  A |  B |  C |  D |  E |  F |  G |  H |
    :----| -: | -: | -: | -: | -: | -: | -: | -: |
    8    | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63 |
    7    | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 |
    6    | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 |
    5    | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 |
    4    | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
    3    | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 |
    2    | 8  | 9  | 10 | 11 | 12 | 13 | 14 | 15 |
    1    | 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  |

    @ingroup board
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

} // namespace chess::board

namespace std {

/** A formatter for chessboard ranks.
    The formatter accepts no arguments; ranks are always printed as integers, except
    starting from 1 instead of 0.

    @see chess::board::Rank
    @ingroup board
 */
template <>
struct formatter<chess::board::Rank> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(
        const chess::board::Rank rank, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}",
            std::to_underlying(rank) + static_cast<chess::board::BitboardIndex>(1));
    }
};

/** A formatter for chessboard files.

    The formatter accepts the following format specifier arguments:
    @li ``u|U``: Tells the formatter to print the file as an uppercase letter
    @li ``l|L``: Tells the formatter to print the file as a lowercase letter

    If no arguments are specified, the formatter prints the rank as an uppercase letter by default.

    @see chess::board::File
    @ingroup board
 */
template <>
struct formatter<chess::board::File> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx);

    template <typename FormatContext>
    typename FormatContext::iterator format(
        chess::board::File file, FormatContext& ctx) const;

private:
    bool uppercase { true };
};

/** A formatter for Square objects.

    The formatter accepts the following format specifier arguments:
    @li ``i|I``: Tells the formatter to print the bitboard bit index for this square
    @li ``a|A``: Tells the formatter to print the algebraic notation of this square

    If no arguments are specified, the formatter prints the square's algebraic notation by default.

    @see chess::board::Square
    @ingroup board
 */
template <>
struct formatter<chess::board::Square> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx);

    template <typename FormatContext>
    typename FormatContext::iterator format(
        const chess::board::Square& square, FormatContext& ctx) const;

private:
    bool asIdx { false };
};

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

/*------------------------ file formatter ------------------------*/

template <typename ParseContext>
constexpr typename ParseContext::iterator
formatter<chess::board::File>::parse(ParseContext& ctx)
{
    auto it = ctx.begin();

    if (it == ctx.end() || *it == '}')
        return it;

    do {
        switch (*it) {
            case 'u': [[fallthrough]];
            case 'U':
                uppercase = true;
                break;

            case 'l': [[fallthrough]];
            case 'L':
                uppercase = false;
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
typename FormatContext::iterator
formatter<chess::board::File>::format(
    const chess::board::File file, FormatContext& ctx) const
{
    const auto character = [file, upper = uppercase] {
        const auto upperChar = magic_enum::enum_name(file).front();

        if (! upper)
            return static_cast<char>(
                std::tolower(static_cast<unsigned char>(upperChar)));

        return upperChar;
    }();

    return std::format_to(ctx.out(), "{}", character);
}

/*------------------------ square formatter ------------------------*/

template <typename ParseContext>
constexpr typename ParseContext::iterator
formatter<chess::board::Square>::parse(ParseContext& ctx)
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
typename FormatContext::iterator
formatter<chess::board::Square>::format(
    const chess::board::Square& square, FormatContext& ctx) const
{
    if (asIdx)
        return std::format_to(ctx.out(), "{}", square.index());

    return std::format_to(
        ctx.out(), "{}{}", square.file, square.rank);
}

} // namespace std

namespace chess::board {

constexpr bool Square::is_light() const noexcept
{
    const auto fileEven = math::is_even(std::to_underlying(file));
    const auto rankEven = math::is_even(std::to_underlying(rank));

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

} // namespace chess::board
