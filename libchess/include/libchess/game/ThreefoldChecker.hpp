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
    This file defines the ThreefoldChecker class.
    @ingroup game
 */

#pragma once

#include <algorithm>
#include <beman/inplace_vector/inplace_vector.hpp>
#include <cstdint> // IWYU pragma: keep - for std::uint64_t
#include <utility>

namespace chess::game {

/** This struct keeps a history of Zobrist hash values to detect threefold repetitions.
    @ingroup game
 */
struct ThreefoldChecker final {
    using HashValue = std::uint64_t;

    /** Clears the history, inserting a single hash value. */
    constexpr void reset(HashValue initialPositionHash);

    /** Pushes a new hash value into the history. */
    constexpr void push(HashValue newHash);

    /** Returns true if the last call to ``push()`` created a threefold repetition in the history. */
    [[nodiscard]] constexpr auto is_threefold() const noexcept -> bool;

    /** Returns the number of times the current position has occurred since the last history reset. */
    [[nodiscard]] constexpr auto repetition_count() const noexcept -> size_t;

private:
    // stores a history of hash values
    // the oldest value is at front() and the most recent is at back()
    // The history is reset by every capture & pawn move, so it only needs to hold the positions
    // since the last one. That's at most 101, since the fifty-move rule draws the game when the
    // halfmove clock reaches 100, so the history never fills up in a real game.
    using History = beman::inplace_vector::inplace_vector<HashValue, 101uz>;

    History history;
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

constexpr void ThreefoldChecker::reset(const HashValue initialPositionHash)
{
    history.clear();
    history.emplace_back(initialPositionHash);
}

constexpr void ThreefoldChecker::push(const HashValue newHash)
{
    // if the history is full, forget the oldest position. That can only happen if more than 100
    // moves are played without a capture or pawn move, which the fifty-move rule doesn't allow
    if (std::cmp_equal(history.size(), History::capacity()))
        history.erase(history.begin());

    history.emplace_back(newHash);
}

constexpr auto ThreefoldChecker::is_threefold() const noexcept -> bool
{
    return std::cmp_greater_equal(
        repetition_count(), 3uz);
}

constexpr auto ThreefoldChecker::repetition_count() const noexcept -> size_t
{
    return static_cast<size_t>(
        std::ranges::count(history, history.back()));
}

} // namespace chess::game
