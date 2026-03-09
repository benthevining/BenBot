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

namespace util {

void progressive_backoff(std::function<bool()> pred)
{
    // measurements on an Apple Silicon Mac or an armv8 based phone

    // stage 1: approx. 2x10 ns (= 20 ns)
    for (auto i = 0uz; i < 2uz; ++i) {
        if (pred())
            return;
    }

    while (true) {
        // stage 2: approx. 750x1333 ns (~ 1 ms)
        for (auto i = 0uz; i < 750uz; ++i) {
            if (pred())
                return;

            __wfe();
        }

        // waiting longer than we should, let's give other threads a chance to recover
        std::this_thread::yield();
    }
}

} // namespace util

#elifdef LIBCHESS_INTEL

#    include <emmintrin.h>

namespace util {

void progressive_backoff(std::function<bool()> pred)
{
    // measurements on a 2.9 GHz Intel i9

    // stage 1: approx. 5x5 ns (= 25 ns)
    for (auto i = 0uz; i < 5uz; ++i) {
        if (pred())
            return;
    }

    // stage 2: approx. 10x40 ns (= 400 ns)
    for (auto i = 0uz; i < 10uz; ++i) {
        if (pred())
            return;

        _mm_pause();
    }

    while (true) {
        // stage 3: approx. 3000x350 ns (~ 1 ms)
        for (auto i = 0uz; i < 3000uz; ++i) {
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

} // namespace util

#else
#    warning "Neither ARM nor Intel detected, using fallback implementation of progressive backoff"

namespace util {

void progressive_backoff(std::function<bool()> pred)
{
    while (not pred())
        std::this_thread::yield();
}

} // namespace util

#endif
