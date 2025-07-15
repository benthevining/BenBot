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
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint64_t

namespace chess::game {

/** This struct keeps a history of Zobrist hash values to detect threefold repetitions.
    @ingroup game
 */
struct ThreefoldChecker final {
    using HashValue = std::uint64_t;

    /** Creates a history containing the given initial hash value. */
    explicit constexpr ThreefoldChecker(HashValue initialPositionHash) noexcept;

    /** Clears the history, inserting a single hash value. */
    constexpr void reset(HashValue initialPositionHash);

    /** Pushes a new hash value into the history. */
    constexpr void push(HashValue newHash);

    /** Returns true if the last call to ``push()`` created a threefold repetition in the history. */
    [[nodiscard]] constexpr bool is_threefold() const noexcept;

private:
    // stores a history of hash values
    // the most recent value is at front() and the oldest is at back()
    beman::inplace_vector<HashValue, 10uz> history;
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

constexpr ThreefoldChecker::ThreefoldChecker(const HashValue initialPositionHash) noexcept
{
    history.emplace_back(initialPositionHash);
}

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
        history.end() - 1,
        history.end());

    history.front() = newHash;
}

constexpr bool ThreefoldChecker::is_threefold() const noexcept
{
    if (history.size() < history.capacity())
        return false;

    // "A" and "B" hashes represent the moves that each player is toggling between
    // when the history does contain a repetition, it looks like this:
    //
    // index | value
    // 0     | ourHashA   <-- this position seen for time #3
    // 1     | theirHashA
    // 2     | ourHashB
    // 3     | theirHashB
    // 4     | ourHashA   <-- this position seen for time #2
    // 5     | theirHashA
    // 6     | ourHashB
    // 7     | theirHashB
    // 8     | ourHashA   <-- this position seen for time #1
    // 9     | theirHashA

    static constexpr auto REP_INC { 4uz };

    for (auto idx = 0uz; idx < REP_INC; ++idx) {
        const auto value = history[idx];

        for (auto repIdx = idx + REP_INC; repIdx < history.capacity(); repIdx += REP_INC)
            if (history[repIdx] != value)
                return false;
    }

    return true;
}

} // namespace chess::game
