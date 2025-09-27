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

/** @defgroup board Board representation
    Classes related to the engine's internal board representation.
    libchess uses bitboards with "Least Significant File" encoding.

    @ingroup libchess
 */

/** @file
    This file defines the Square class and related functions.

    @ingroup board
 */

#pragma once

#include <cassert>
#include <compare>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <expected>
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/util/Math.hpp>
#include <string>
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

        If the input string cannot be parsed correctly, returns an explanatory error string.
     */
    [[nodiscard]] static std::expected<Square, std::string> from_string(std::string_view text);

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
    [[nodiscard]] constexpr bool is_dark() const noexcept { return not is_light(); }
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

/** Given the en passant target square, this returns the square that the
    captured pawn was on.
 */
[[nodiscard, gnu::const]] constexpr Square get_en_passant_captured_square(
    const Square& targetSquare, bool isWhite) noexcept;

/// @}

} // namespace chess::board

/** A formatter specialization for Square objects.
    The formatter accepts no arguments; squares are always printed in algebraic notation,
    such as ``a4``, ``b2``, etc.

    @see chess::board::Square
    @ingroup board
 */
template <>
struct std::formatter<chess::board::Square> final {
    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(
        const chess::board::Square& square, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}{}", square.file, square.rank);
    }
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
    return static_cast<BitboardIndex>(
        (static_cast<int>(rank) << 3) + static_cast<int>(file));
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
    return not util::is_even(
        std::to_underlying(rank) + std::to_underlying(file));
}

inline std::expected<Square, std::string> Square::from_string(const std::string_view text)
{
    if (text.length() != 2uz) {
        return std::unexpected(
            std::format(
                "Cannot parse Square from invalid input string: {}",
                text));
    }

    const auto rank = rank_from_char(text.back());

    if (not rank.has_value())
        return std::unexpected(rank.error());

    const auto file = file_from_char(text.front());

    if (not file.has_value())
        return std::unexpected(file.error());

    return Square {
        .file = file.value(),
        .rank = rank.value()
    };
}

constexpr Square get_en_passant_captured_square(
    const Square& targetSquare, const bool isWhite) noexcept
{
    // the captured pawn is on the file of the target square, but
    // one rank below (White capture) or one rank above (Black capture)
    const auto capturedRank = isWhite
                                ? prev_pawn_rank<Color::White>(targetSquare.rank)
                                : prev_pawn_rank<Color::Black>(targetSquare.rank);

    return Square {
        .file = targetSquare.file,
        .rank = capturedRank
    };
}

} // namespace chess::board
