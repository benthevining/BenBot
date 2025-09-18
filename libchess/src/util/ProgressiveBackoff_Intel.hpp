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

#pragma once

#include <emmintrin.h>
#include <functional>
#include <thread>

namespace chess::util {

inline void progressive_backoff_impl(std::function<bool()> pred)
{
    // approx. 5x5 ns (= 25 ns), 10x40 ns (= 400 ns), and 3000x350 ns (~ 1 ms),
    // respectively, when measured on a 2.9 GHz Intel i9
    static constexpr auto N0 = 5uz;    // NOLINT(readability-identifier-length)
    static constexpr auto N1 = 10uz;   // NOLINT(readability-identifier-length)
    static constexpr auto N2 = 3000uz; // NOLINT(readability-identifier-length)

    for (auto i = 0uz; i < N0; ++i) {
        if (pred())
            return;
    }

    for (auto i = 0uz; i < N1; ++i) {
        if (pred())
            return;

        _mm_pause();
    }

    while (true) {
        for (auto i = 0uz; i < N2; ++i) {
            if (pred())
                return;

            // do not roll these into a loop: not every compiler unrolls it
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
            _mm_pause();
        }

        // waiting longer than we should, let's give other threads a chance to recover
        std::this_thread::yield();
    }
}

} // namespace chess::util
