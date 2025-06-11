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
    This file defines the Square class and related functions.

    @ingroup board
 */

#pragma once

#include <cassert>
#include <compare>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/util/Math.hpp>
#include <stdexcept>
#include <string_view>
#include <utility>

/** This namespace contains classes related to the engine's internal
    board representation.

    @ingroup board
 */
namespace chess::board {

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
     */
    [[nodiscard, gnu::const]] static constexpr Square from_index(BitboardIndex index) noexcept;

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
    [[nodiscard]] constexpr BitboardIndex index() const noexcept;

    /** Returns true if two squares are equivalent. */
    [[nodiscard]] constexpr bool operator==(const Square&) const noexcept = default;

    /// @name Area queries
    /// @{

    /** Returns true if this square is on the queenside (the A-D files). */
    [[nodiscard]] constexpr bool is_queenside() const noexcept;

    /** Returns true if this square is on the kingside (the E-H files). */
    [[nodiscard]] constexpr bool is_kingside() const noexcept;

    /** Returns true if this square is within White's territory (the first through fourth ranks). */
    [[nodiscard]] constexpr bool is_white_territory() const noexcept;

    /** Returns true if this square is within Black's territory (the fifth through eighth ranks). */
    [[nodiscard]] constexpr bool is_black_territory() const noexcept;

    /// @}

    /** Returns true if this is a light square. */
    [[nodiscard]] constexpr bool is_light() const noexcept;

    /** Returns true if this is a dark square. */
    [[nodiscard]] constexpr bool is_dark() const noexcept { return ! is_light(); }
};

/// @ingroup board
/// @{

/** Orders the two squares based on their bitboard indices.
    @relates Square
 */
[[nodiscard, gnu::const]] constexpr std::strong_ordering operator<=>(
    const Square& first, const Square& second) noexcept
{
    return first.index() <=> second.index();
}

/// @}

} // namespace chess::board

namespace std {

/** A formatter specialization for Square objects.

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

constexpr Square Square::from_index(const BitboardIndex index) noexcept
{
    assert(index <= MAX_BITBOARD_IDX);

    return {
        .file = static_cast<File>(index & static_cast<BitboardIndex>(7)),
        .rank = static_cast<Rank>(index >> static_cast<BitboardIndex>(3))
    };
}

constexpr BitboardIndex Square::index() const noexcept
{
    return (std::to_underlying(rank) << static_cast<BitboardIndex>(3)) + std::to_underlying(file);
}

constexpr bool Square::is_queenside() const noexcept
{
    return std::cmp_less_equal(
        std::to_underlying(file), std::to_underlying(File::D));
}

constexpr bool Square::is_kingside() const noexcept
{
    return std::cmp_greater_equal(
        std::to_underlying(file), std::to_underlying(File::E));
}

constexpr bool Square::is_white_territory() const noexcept
{
    return std::cmp_less_equal(
        std::to_underlying(rank), std::to_underlying(Rank::Four));
}

constexpr bool Square::is_black_territory() const noexcept
{
    return std::cmp_greater_equal(
        std::to_underlying(rank), std::to_underlying(Rank::Five));
}

constexpr bool Square::is_light() const noexcept
{
    return ! util::is_even(
        std::to_underlying(rank) + std::to_underlying(file));
}

constexpr Square Square::from_string(const std::string_view text)
{
    if (text.length() != 2uz)
        throw std::invalid_argument {
            std::format("Cannot parse Square from invalid input string: {}", text)
        };

    return {
        .file = file_from_char(text.front()),
        .rank = rank_from_char(text.back())
    };
}

} // namespace chess::board
