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
    This file defines the opening book data structure.
    @ingroup search
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/PGN.hpp>
#include <libchess/uci/DefaultOptions.hpp>
#include <libchess/uci/Options.hpp>
#include <optional>
#include <print>
#include <random>
#include <span>
#include <unordered_map>
#include <vector>

namespace chess::search {

using game::Position;
using moves::Move;
using std::size_t;

/** The opening book data structure.

    This data structure maps positions to possible moves.

    @ingroup search
    @see OpeningBookContext
 */
class OpeningBook final {
public:
    /** Returns the possible continuations for the given position.
        Returns an empty span if the given position wasn't found in the book.
     */
    [[nodiscard]] std::span<const Move> get_moves(const Position& position) const;

    /** Adds moves from a PGN file. */
    void add_from_pgn(
        const notation::GameRecord& game,
        const bool                  includeVariations = true)
    {
        add_pgn_moves(game.moves, game.startingPosition, includeVariations);
        prune();
    }

    /** Writes the openings database as a set of PGNs.
        Each root move from the starting position is given its own PGN file,
        with variations being used to consolidate them from there.
     */
    [[nodiscard]] std::vector<notation::GameRecord> to_pgns() const;

    /** Prunes duplicate moves from the database. */
    void prune();

    void print_stats() const
    {
        std::println("Num stored positions: {}", lines.size());

        const Position startPos;

        std::println("Num moves from start pos: {}", get_moves(startPos).size());
    }

private:
    void add_pgn_moves(
        std::span<const notation::GameRecord::Move> moves,
        Position position, bool includeVariations);

    void add_next_position(
        const Position&       position,
        notation::GameRecord& game) const;

    std::unordered_map<Position::Hash, std::vector<Move>> lines;
};

/** This struct combines an opening book database with a parameter to enable/disable it,
    as well as a random number generator used for choosing between possible continuations.

    @ingroup search
    @see OpeningBook
 */
struct OpeningBookContext final {
    /** The opening book itself. */
    OpeningBook book;

    /** This parameter controls whether the engine is allowed to access its internal book. */
    uci::BoolOption enabled { uci::default_options::own_book() };

    /** Returns a book move for the given position. Returns nullopt if the book is
        disabled, or if the given position is out-of-book. If the book is enabled
        and the openings database returns multiple possible continuations for the
        given position, one move is chosen at random using an internal RNG.
     */
    [[nodiscard]] std::optional<Move> get_move(const Position& position);

private:
    std::mt19937_64 rng;

    std::uniform_int_distribution<size_t> dist { 0uz };
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

inline std::span<const Move> OpeningBook::get_moves(const Position& position) const
{
    if (const auto it = lines.find(position.hash);
        it != lines.end()) {
        return it->second;
    }

    return {};
}

inline std::optional<Move> OpeningBookContext::get_move(const Position& position)
{
    if (! enabled.get_value())
        return std::nullopt;

    const auto moves = book.get_moves(position);

    if (moves.empty())
        return std::nullopt;

    const auto idx = dist(rng) % moves.size();

    return moves[idx];
}

} // namespace chess::search
