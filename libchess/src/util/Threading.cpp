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

#if defined(LIBCHESS_ARM) && defined(LIBCHESS_INTEL)
#    error "Both ARM and Intel detected!"
#endif

#ifdef LIBCHESS_ARM
#    include "ProgressiveBackoff_ARM.hpp"
#elifdef LIBCHESS_INTEL
#    include "ProgressiveBackoff_Intel.hpp"
#else
#    include <thread>

namespace {
void progressive_backoff_impl(std::function<bool()> pred)
{
    while (not pred())
        std::this_thread::yield();
}
} // namespace
#endif

namespace chess::util {

void progressive_backoff(std::function<bool()> pred)
{
    progressive_backoff_impl(std::move(pred));
}

} // namespace chess::util
