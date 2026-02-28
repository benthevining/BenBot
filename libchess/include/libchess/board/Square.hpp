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
    [[nodiscard, gnu::const]] static constexpr auto from_index(BitboardIndex index) noexcept -> Square;

    /** Creates a square from a string in algebraic notation, such as "A1", "H4", etc.

        This method recognizes either upper- or lower-case file letters.

        If the input string cannot be parsed correctly, returns an explanatory error string.
     */
    [[nodiscard]] static auto from_string(std::string_view text) -> std::expected<Square, std::string>;

    /** Returns the bitboard bit index for this square.
        The returned index will be in the range ``[0,63]``.
     */
    [[nodiscard]] constexpr auto index() const noexcept -> BitboardIndex;

    /** Returns true if two squares are equivalent. */
    [[nodiscard]] constexpr auto operator==(const Square&) const noexcept -> bool = default;

    /// @name Area queries
    /// @{

    /** Returns true if this square is on the queenside (the A-D files). */
    [[nodiscard]] constexpr auto is_queenside() const noexcept -> bool;

    /** Returns true if this square is on the kingside (the E-H files). */
    [[nodiscard]] constexpr auto is_kingside() const noexcept -> bool;

    /** Returns true if this square is within White's territory (the first through fourth ranks). */
    [[nodiscard]] constexpr auto is_white_territory() const noexcept -> bool;

    /** Returns true if this square is within Black's territory (the fifth through eighth ranks). */
    [[nodiscard]] constexpr auto is_black_territory() const noexcept -> bool;

    /// @}

    /** Returns true if this is a light square. */
    [[nodiscard]] constexpr auto is_light() const noexcept -> bool;

    /** Returns true if this is a dark square. */
    [[nodiscard]] constexpr auto is_dark() const noexcept -> bool { return not is_light(); }
};

/// @ingroup board
/// @{

/** Orders the two squares based on their bitboard indices.
    @relates Square
 */
[[nodiscard, gnu::const]] constexpr auto operator<=>(
    const Square& first, const Square& second) noexcept
    -> std::strong_ordering
{
    return first.index() <=> second.index();
}

/** Given the en passant target square, this returns the square that the
    captured pawn was on.
 */
[[nodiscard, gnu::const]] constexpr auto get_en_passant_captured_square(
    Square targetSquare, bool isWhite) noexcept
    -> Square;

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
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(
        const chess::board::Square& square, FormatContext& ctx) const
        -> typename FormatContext::iterator
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

constexpr auto Square::from_index(const BitboardIndex index) noexcept -> Square
{
    assert(std::cmp_less_equal(index, MAX_BITBOARD_IDX));

    return {
        .file = static_cast<File>(index & static_cast<BitboardIndex>(7)),
        .rank = static_cast<Rank>(index >> static_cast<BitboardIndex>(3))
    };
}

constexpr auto Square::index() const noexcept -> BitboardIndex
{
    return static_cast<BitboardIndex>(
        (static_cast<int>(rank) << 3) + static_cast<int>(file));
}

constexpr auto Square::is_queenside() const noexcept -> bool
{
    return std::cmp_less_equal(
        std::to_underlying(file), std::to_underlying(File::D));
}

constexpr auto Square::is_kingside() const noexcept -> bool
{
    return std::cmp_greater_equal(
        std::to_underlying(file), std::to_underlying(File::E));
}

constexpr auto Square::is_white_territory() const noexcept -> bool
{
    return std::cmp_less_equal(
        std::to_underlying(rank), std::to_underlying(Rank::Four));
}

constexpr auto Square::is_black_territory() const noexcept -> bool
{
    return std::cmp_greater_equal(
        std::to_underlying(rank), std::to_underlying(Rank::Five));
}

constexpr auto Square::is_light() const noexcept -> bool
{
    return not util::math::is_even(
        std::to_underlying(rank) + std::to_underlying(file));
}

inline auto Square::from_string(const std::string_view text)
    -> std::expected<Square, std::string>
{
    if (std::cmp_not_equal(text.length(), 2)) {
        return std::unexpected {
            std::format(
                "Cannot parse Square from invalid input string: {}",
                text)
        };
    }

    return rank_from_char(text.back())
        .and_then([fileChar = text.front()](const Rank rankToUse) {
            return file_from_char(fileChar)
                .transform([rankToUse](const File fileToUse) {
                    return Square {
                        .file = fileToUse,
                        .rank = rankToUse
                    };
                });
        });
}

constexpr auto get_en_passant_captured_square(
    const Square targetSquare, const bool isWhite) noexcept
    -> Square
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
