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
#include <iterator>
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

private:
    // stores a history of hash values
    // the most recent value is at front() and the oldest is at back()
    beman::inplace_vector::inplace_vector<HashValue, 50uz> history;
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
    // make room for new element
    if (history.size() < history.capacity())
        history.emplace_back(0uz);

    // move the last element to the front,
    // and move all other elements back 1
    std::ranges::rotate(
        history.begin(),
        std::prev(history.end()),
        history.end());

    history.front() = newHash;
}

constexpr auto ThreefoldChecker::is_threefold() const noexcept -> bool
{
    // NB. the threefold repetition doesn't need to be consecutive within the history
    return std::cmp_greater_equal(
        std::ranges::count(history, history.front()),
        3uz);
}

} // namespace chess::game
