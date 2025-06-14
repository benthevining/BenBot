/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

/** @file
    This file provides an algorithm for deciding how much time to search.
    @ingroup game
 */

#pragma once

#include <chrono>

namespace chess::game {

using Milliseconds = std::chrono::milliseconds;

/** Determines how long to cap the next search at, based on our remaining
    time and increment.

    @ingroup game
 */
[[nodiscard, gnu::const]] constexpr Milliseconds determine_search_time(
    const Milliseconds& timeRemaining, const Milliseconds& increment) noexcept;

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

constexpr Milliseconds determine_search_time(
    const Milliseconds& timeRemaining, const Milliseconds& increment) noexcept
{
    return Milliseconds {
        (timeRemaining.count() / 20uz)
        + (increment.count() / 2uz)
    };
}

} // namespace chess::game
