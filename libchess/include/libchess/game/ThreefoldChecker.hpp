/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
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
#include <libchess/game/Zobrist.hpp>

namespace chess::game {

/** This struct keeps a history of Zobrist hash values to detect threefold repetitions.
    @ingroup game
 */
struct ThreefoldChecker final {
    using HashValue = zobrist::Value;

    /** Creates a history containing the given initial hash value. */
    explicit constexpr ThreefoldChecker(HashValue initialPositionHash = 0uz) noexcept;

    /** Clears the history, inserting a single hash value. */
    constexpr void reset(HashValue initialPositionHash) noexcept;

    /** Pushes a new hash value into the history. */
    constexpr void push(HashValue newHash) noexcept;

    /** Returns true if the last call to ``push()`` created a threefold repetition in the history. */
    [[nodiscard]] constexpr bool is_threefold() const noexcept;

private:
    // stores the hash values for the last 6 plies
    // the most recent value is at front() and the oldest is at back()
    std::array<HashValue, 6uz> history {};
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

constexpr void ThreefoldChecker::reset(const HashValue initialPositionHash) noexcept
{
    for (auto idx = 1uz; idx < history.size(); ++idx)
        history[idx] = 0uz;

    history.front() = initialPositionHash;
}

constexpr void ThreefoldChecker::push(const HashValue newHash) noexcept
{
    // move all elements back
    for (auto idx = history.size() - 1uz; idx > 0uz; --idx)
        history[idx] = history[idx - 1uz];

    history.front() = newHash;
}

constexpr bool ThreefoldChecker::is_threefold() const noexcept
{
    const auto hash = history.front();

    std::size_t repeats { 0uz };

    for (auto idx = 2uz; idx <= history.size(); idx += 2uz) {
        if (history[history.size() - idx] == hash) {
            ++repeats;

            if (repeats >= 2uz)
                return true;
        }
    }

    return false;
}

} // namespace chess::game
