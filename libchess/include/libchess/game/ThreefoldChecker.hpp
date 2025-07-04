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

#include <array>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdint> // IWYU pragma: keep - for std::uint64_t

namespace chess::game {

/** This struct keeps a history of Zobrist hash values to detect threefold repetitions.

    @invariant There should always be at least one non-zero hash value in the history,
    otherwise a history of all zero hash values would erroneously be detected as a
    threefold repetition.

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
    std::array<HashValue, 10uz> history {};
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
    history.front() = initialPositionHash;
}

constexpr void ThreefoldChecker::reset(const HashValue initialPositionHash)
{
    for (auto idx = 1uz; idx < history.size(); ++idx)
        history.at(idx) = 0uz;

    history.front() = initialPositionHash;
}

constexpr void ThreefoldChecker::push(const HashValue newHash)
{
    // move all elements back
    for (auto idx = history.size() - 1uz; idx > 0uz; --idx)
        history.at(idx) = history.at(idx - 1uz);

    history.front() = newHash;
}

constexpr bool ThreefoldChecker::is_threefold() const noexcept
{
    // "A" and "B" hashes represent the moves that each player is toggling between

    const auto ourHashA   = history.front();
    const auto theirHashA = history[1uz];

    const auto ourHashB   = history[2uz];
    const auto theirHashB = history[3uz];

    return ourHashA == history[4uz]
        && theirHashA == history[5uz]
        && ourHashB == history[6uz]
        && theirHashB == history[7uz]
        && ourHashA == history[8uz]
        && theirHashA == history[9uz];
}

} // namespace chess::game
