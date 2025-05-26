/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file defines the CastlingRights struct.
    @ingroup game
 */

#pragma once

namespace chess::game {

/** This struct encapsulates one side's castling rights.

    @ingroup game
 */
struct CastlingRights final {
    /** True if kingside (or "short") castling is available. */
    bool kingside { true };

    /** True if queenside (or "long") castling is available. */
    bool queenside { true };

    /** Returns true if the two sets of castling rights are identical. */
    [[nodiscard]] constexpr bool operator==(const CastlingRights&) const noexcept = default;

    /** Called when the king moves. */
    constexpr void king_moved() noexcept
    {
        kingside  = false;
        queenside = false;
    }
};

} // namespace chess::game
