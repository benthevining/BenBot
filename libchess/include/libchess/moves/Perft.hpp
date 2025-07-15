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
    This file defines a perft function using our move generation code.
    @ingroup moves
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t;
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <utility>
#include <vector>

namespace chess::moves {

using std::size_t;

/** Results from a perft computation.

    @ingroup moves
    @see perft()
 */
struct PerftResult final {
    /** The total number of nodes in the move tree. */
    size_t nodes { 0uz };

    /** The number of leaf nodes that are captures. */
    size_t captures { 0uz };

    /** The number of leaf nodes that are en passant captures. */
    size_t enPassantCaptures { 0uz };

    /** The number of leaf nodes that are castling. */
    size_t castles { 0uz };

    /** The number of leaf nodes that are promotions. */
    size_t promotions { 0uz };

    /** The number of leaf nodes that are checks. */
    size_t checks { 0uz };

    /** The number of leaf nodes that are checkmates. */
    size_t checkmates { 0uz };

    /** The number of leaf nodes that are stalemates. */
    size_t stalemates { 0uz };

    /** A pair of a move and its number of child nodes. */
    using RootNodeInfo = std::pair<moves::Move, size_t>;

    /** For each of the root moves from the starting position,
        this records how many child nodes are under that move's
        branch of the tree.
     */
    std::vector<RootNodeInfo> rootNodes;

    /** Adds a child result to this one. */
    constexpr PerftResult& operator+=(const PerftResult& rhs) noexcept;
};

/** Computes perft of the given position.

    @ingroup moves
    @relates PerftResult
 */
template <bool IsRoot = true>
[[nodiscard]] PerftResult perft(
    size_t                depth,
    const game::Position& startingPosition);

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

constexpr PerftResult& PerftResult::operator+=(const PerftResult& rhs) noexcept
{
    nodes += rhs.nodes;
    captures += rhs.captures;
    enPassantCaptures += rhs.enPassantCaptures;
    castles += rhs.castles;
    promotions += rhs.promotions;
    checks += rhs.checks;
    checkmates += rhs.checkmates;
    stalemates += rhs.stalemates;

    return *this;
}

template <bool IsRoot>
PerftResult perft(
    const size_t          depth,
    const game::Position& startingPosition)
{
    if (depth == 0uz)
        return { .nodes = 1uz };

    PerftResult result;

    for (const auto& move : generate(startingPosition)) {
        const auto newPosition = after_move(startingPosition, move);

        // we want stats only for leaf nodes
        if (depth == 1uz) {
            if (startingPosition.is_capture(move)) {
                ++result.captures;

                if (startingPosition.is_en_passant(move))
                    ++result.enPassantCaptures;
            }

            if (move.is_castling())
                ++result.castles;

            if (move.promotedType.has_value())
                ++result.promotions;

            const bool isCheck = newPosition.is_check();

            if (isCheck)
                ++result.checks;

            if (not any_legal_moves(newPosition)) {
                if (isCheck)
                    ++result.checkmates;
                else
                    ++result.stalemates;
            }
        }

        const auto childResult = perft<false>(depth - 1uz, newPosition);

        if constexpr (IsRoot) {
            result.rootNodes.emplace_back(move, childResult.nodes);
        }

        result += childResult;
    }

    return result;
}

} // namespace chess::moves
