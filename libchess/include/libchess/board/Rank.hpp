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

#include <format>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/pieces/Colors.hpp>

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
[[nodiscard, gnu::const]] constexpr Rank back_rank_for(const Color color) noexcept
{
    if (color == Color::White)
        return Rank::One;

    return Rank::Eight;
}

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
            std::to_underlying(rank) + static_cast<chess::board::BitboardIndex>(1));
    }
};
