/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the Rank enumeration.
    @ingroup board
 */

#pragma once

#include <cassert>
#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/Colors.hpp>
#include <stdexcept>
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
[[nodiscard, gnu::const]] constexpr Rank back_rank_for(Color color) noexcept;

/** Returns the next pawn rank, from the given side's perspective. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Rank next_pawn_rank(Rank rank) noexcept;

/** Returns the previous pawn rank, from the given side's perspective. */
template <Color Side>
[[nodiscard, gnu::const]] constexpr Rank prev_pawn_rank(Rank rank) noexcept;

/** Interprets the given character as a rank.

    @throws std::invalid_argument An exception will be thrown if a rank
    cannot be parsed correctly from the input character.

    @ingroup board
    @see Rank
 */
[[nodiscard, gnu::const]] constexpr Rank rank_from_char(char character);

/** Converts the rank to its single-character representation (as an integer).

    @ingroup board
    @see Rank
 */
[[nodiscard, gnu::const]] constexpr char rank_to_char(Rank rank) noexcept;

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
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    typename FormatContext::iterator format(
        const chess::board::Rank rank, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}",
            chess::board::rank_to_char(rank));
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

constexpr Rank back_rank_for(const Color color) noexcept
{
    if (color == Color::White)
        return Rank::One;

    return Rank::Eight;
}

template <Color Side>
constexpr Rank next_pawn_rank(const Rank rank) noexcept
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
constexpr Rank prev_pawn_rank(const Rank rank) noexcept
{
    if constexpr (Side == Color::White) {
        assert(rank != Rank::One);
        return static_cast<Rank>(std::to_underlying(rank) - 1uz);
    } else {
        assert(rank != Rank::Eight);
        return static_cast<Rank>(std::to_underlying(rank) + 1uz);
    }
}

constexpr Rank rank_from_char(const char character)
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
            throw std::invalid_argument {
                std::format("Cannot parse Rank from character: {}", character)
            };
    }
}

constexpr char rank_to_char(const Rank rank) noexcept
{
    static constexpr std::string_view ranks { "12345678" };

    return ranks[std::to_underlying(rank)];
}

} // namespace chess::board
