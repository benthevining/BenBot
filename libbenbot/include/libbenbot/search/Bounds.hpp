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
    This file defines the search bounds struct.
    @ingroup search
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/eval/Score.hpp>
#include <optional>

namespace ben_bot::search {

using eval::Score;
using std::size_t;

/** This struct encapsulates a search's alpha and beta bounds values.
    This struct is mostly for internal usage by the search context class.

    @ingroup search
 */
struct Bounds final {
    /** The search's alpha bound. */
    Score alpha { -eval::MAX };

    /** The search's beta bound. */
    Score beta { eval::MAX };

    /** Returns an inverted copy of these bounds, suitable for swapping the
        player perspectives.
     */
    [[nodiscard]] constexpr auto invert() const noexcept -> Bounds;

    /** Returns a null window around alpha. For usage with principal variation
        search.
     */
    [[nodiscard]] constexpr auto null_window() const noexcept -> Bounds;

    /** Returns a null window around beta, from the opponent's perspective. For
        usage with null move pruning.
     */
    [[nodiscard]] constexpr auto null_window_beta() const noexcept -> Bounds;

    /** Returns true if the score is between alpha and beta. */
    [[nodiscard]] constexpr auto contains(Score score) const noexcept -> bool;

    /** Performs mate distance pruning.
        If an MDP cutoff is available, returns the cutoff value (and the
        search may return early). If no MDP cutoff is available, this method
        returns ``nullopt`` and the search should continue.
     */
    [[nodiscard]] constexpr auto mate_distance_pruning(size_t plyFromRoot) noexcept -> std::optional<Score>;
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

constexpr auto Bounds::invert() const noexcept -> Bounds
{
    return {
        .alpha = -beta,
        .beta  = -alpha
    };
}

constexpr auto Bounds::null_window() const noexcept -> Bounds
{
    return {
        .alpha = Score { static_cast<eval::Value>(-alpha.value - UINT16_C(1)) },
        .beta  = -alpha
    };
}

constexpr auto Bounds::null_window_beta() const noexcept -> Bounds
{
    return {
        .alpha = -beta,
        .beta  = Score { static_cast<eval::Value>(-beta.value + UINT16_C(1)) }
    };
}

constexpr auto Bounds::contains(const Score score) const noexcept -> bool
{
    return score > alpha and score < beta;
}

constexpr auto Bounds::mate_distance_pruning(const size_t plyFromRoot) noexcept -> std::optional<Score>
{
    // the worst we can do from here is be mated in this position,
    // and the best we can do is deliver mate on our next move
    const auto matedScore = Score::mate(plyFromRoot);
    const auto mateScore  = -Score::mate(plyFromRoot + 1uz);

    if (alpha < matedScore)
        alpha = matedScore;

    if (beta > mateScore)
        beta = mateScore;

    if (alpha >= beta)
        return alpha;

    return std::nullopt;
}

} // namespace ben_bot::search
