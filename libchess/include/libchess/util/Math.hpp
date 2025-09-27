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

/** @defgroup util General utilities
    General utilities not specific to chess, but used by libchess.

    @ingroup libchess
 */

/** @file
    This file provides some basic maths utility functions.
    @ingroup util
 */

#pragma once

#include <concepts>
#include <cstdint> // IWYU pragma: keep - for std::uint64_t

#if defined(_M_X64) || defined(_M_ARM64) || defined(_M_IA64)
#    include <intrin.h>
#endif

/** This namespace provides general utilities not specific to chess.
    @ingroup util
 */
namespace chess::util {

/** Returns true if the given value is an even number.
    @ingroup util
 */
[[nodiscard, gnu::const]] constexpr auto is_even(
    const std::integral auto value) noexcept
    -> bool
{
    return static_cast<int>(value) % 2 == 0;
}

using std::uint64_t;

/** Multiplies the two integers and returns the highest 64 bits of the
    128-bit result as a 64-bit integer.
    @ingroup util
 */
[[nodiscard, gnu::const]] inline auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
#ifdef __SIZEOF_INT128__
    __extension__ using uint128_t = unsigned __int128;

    return (static_cast<uint128_t>(first) * static_cast<uint128_t>(second)) >> static_cast<uint128_t>(64);
#elif defined(_M_X64) || defined(_M_ARM64) // MSVC for x86-64 or AArch64
    return __umulh(first, second);
#elifdef _M_IA64
    // https://learn.microsoft.com/en-gb/cpp/intrinsics/umul128
    // incorrectly say that _umul128 is available for ARM
    unsigned __int64 highProduct { 0 };
    _umul128(a, b, &highProduct);
    return highProduct;
#else
#    warning "No optimized version of mul_hi64() is available, using fallback"

    auto get_lo_32_bits = [](const uint64_t value) {
        return static_cast<uint64_t>(static_cast<std::uint32_t>(value));
    };

    const auto also = get_lo_32_bits(first);
    const auto aHi  = first >> 32uz;

    const auto bLo = get_lo_32_bits(second);
    const auto bHi = second >> 32uz;

    const auto c1 = (also * bLo) >> 32uz;              // NOLINT(readability-identifier-length)
    const auto c2 = (aHi * bLo) + c1;                  // NOLINT(readability-identifier-length)
    const auto c3 = (also * bHi) + get_lo_32_bits(c2); // NOLINT(readability-identifier-length)

    return (aHi * bHi) + (c2 >> 32uz) + (c3 >> 32uz);
#endif
}

} // namespace chess::util
