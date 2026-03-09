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
    This file defines the Rank enumeration.
    @ingroup board
 */

#pragma once

#include <cassert>
#include <expected>
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/Colors.hpp>
#include <string>
#include <string_view>
#include <utility>

namespace chess::board {

using pieces::Color;

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

/** Returns the back rank for the given side.
    A player's back rank is the rank their king starts on.

    @ingroup board
 */
[[nodiscard, gnu::const]] constexpr auto back_rank_for(Color color) noexcept -> Rank;

/** Returns the next pawn rank, from the given side's perspective.
    @ingroup board
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto next_pawn_rank(Rank rank) noexcept -> Rank;

/** Returns the previous pawn rank, from the given side's perspective.
    @ingroup board
 */
template <Color Side>
[[nodiscard, gnu::const]] constexpr auto prev_pawn_rank(Rank rank) noexcept -> Rank;

/** Interprets the given character as a rank.

    If the input string cannot be parsed correctly, returns an explanatory error string.

    @ingroup board
    @see Rank
 */
[[nodiscard]] auto rank_from_char(char character)
    -> std::expected<Rank, std::string>;

/** Converts the rank to its single-character representation (as an integer).

    @ingroup board
    @see Rank
 */
[[nodiscard, gnu::const]] constexpr auto rank_to_char(Rank rank) -> char;

} // namespace chess::board

/** A formatter specialization for chessboard ranks.
    The formatter accepts no arguments; ranks are always printed as integers, except
    starting from 1 instead of 0.

    @see chess::board::Rank
    @ingroup board
 */
template <>
struct std::formatter<chess::board::Rank> final {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(
        const chess::board::Rank rank, FormatContext& ctx) const
        -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(), "{}", rank_to_char(rank));
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

constexpr auto back_rank_for(const Color color) noexcept -> Rank
{
    if (color == Color::White)
        return Rank::One;

    return Rank::Eight;
}

template <Color Side>
constexpr auto next_pawn_rank(const Rank rank) noexcept -> Rank
{
    if constexpr (Side == Color::White) {
        assert(rank != Rank::Eight);
        return static_cast<Rank>(std::to_underlying(rank) + 1uz);
    } else {
        assert(rank != Rank::One);
        return static_cast<Rank>(std::to_underlying(rank) - 1uz);
    }
}

template <Color Side>
constexpr auto prev_pawn_rank(const Rank rank) noexcept -> Rank
{
    if constexpr (Side == Color::White) {
        assert(rank != Rank::One);
        return static_cast<Rank>(std::to_underlying(rank) - 1uz);
    } else {
        assert(rank != Rank::Eight);
        return static_cast<Rank>(std::to_underlying(rank) + 1uz);
    }
}

inline auto rank_from_char(const char character)
    -> std::expected<Rank, std::string>
{
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
            return std::unexpected {
                std::format(
                    "Cannot parse Rank from character: {}",
                    character)
            };
    }
}

constexpr auto rank_to_char(const Rank rank) -> char
{
    constexpr std::string_view ranks { "12345678" };

    return ranks.at(std::to_underlying(rank));
}

} // namespace chess::board
