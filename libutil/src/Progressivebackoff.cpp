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

#include "DetectArch.hpp" // NOLINT(build/include_subdir)
#include <functional>
#include <libutil/Threading.hpp>
#include <thread>

#ifdef LIBCHESS_ARM

#    include <arm_acle.h>

namespace chess::util {

void progressive_backoff(std::function<bool()> pred)
{
    // approx. 2x10 ns (= 20 ns) and 750x1333 ns (~ 1 ms), respectively,
    // on an Apple Silicon Mac or an armv8 based phone
    static constexpr auto N0 = 2uz;   // NOLINT(readability-identifier-length)
    static constexpr auto N1 = 750uz; // NOLINT(readability-identifier-length)

    for (auto i = 0uz; i < N0; ++i) {
        if (pred())
            return;
    }

    while (true) {
        for (auto i = 0uz; i < N1; ++i) {
            if (pred())
                return;

            __wfe();
        }

        // waiting longer than we should, let's give other threads a chance to recover
        std::this_thread::yield();
    }
}

} // namespace chess::util

#elifdef LIBCHESS_INTEL

#    include <emmintrin.h>

namespace chess::util {

void progressive_backoff(std::function<bool()> pred)
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

#else
#    warning "Neither ARM nor Intel detected, using fallback implementation of progressive backoff"

namespace chess::util {

void progressive_backoff(std::function<bool()> pred)
{
    while (not pred())
        std::this_thread::yield();
}

} // namespace chess::util

#endif
