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

#include <functional>
#include <libchess/util/Threading.hpp>
#include <utility>

#ifndef LIBCHESS_ARM
#    if defined(__arm__) || defined(__arm64__)
#        define LIBCHESS_ARM 1
#    endif
#endif

#ifndef LIBCHESS_INTEL
#    if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#        define LIBCHESS_INTEL 1
#    endif
#endif

#ifdef LIBCHESS_ARM
#    include "ProgressiveBackoff_ARM.hpp"
#elifdef LIBCHESS_INTEL
#    include "ProgressiveBackoff_Intel.hpp"
#else
#    include <thread>
#endif

namespace chess::util {

void progressive_backoff(std::function<bool()> pred)
{
#ifdef LIBCHESS_ARM
    // approx. 2x10 ns (= 20 ns) and 750x1333 ns (~ 1 ms), respectively, on an
    // Apple Silicon Mac or an armv8 based phone.
    progressive_backoff_arm<2uz, 750uz>(std::move(pred));
#elifdef LIBCHESS_INTEL
    // approx. 5x5 ns (= 25 ns), 10x40 ns (= 400 ns), and 3000x350 ns (~ 1 ms),
    // respectively, when measured on a 2.9 GHz Intel i9
    progressive_backoff_intel<5uz, 10uz, 3000uz>(std::move(pred)); // cppcheck-suppress accessMoved
#else
#    warning "Not Intel or ARM, using naive implementation of progressive_backoff()"

    while (not pred())
        std::this_thread::yield();
#endif
}

} // namespace chess::util
