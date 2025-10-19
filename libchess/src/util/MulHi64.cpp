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

#include <libchess/util/Math.hpp>

#ifdef __SIZEOF_INT128__

namespace chess::util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    __extension__ using uint128_t = unsigned __int128;

    return (static_cast<uint128_t>(first) * static_cast<uint128_t>(second)) >> static_cast<uint128_t>(64);
}

} // namespace chess::util::math

#elif defined(_M_X64) or defined(_M_ARM64) // MSVC for x86-64 or AArch64

#    include <intrin.h>

namespace chess::util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    return __umulh(first, second);
}

} // namespace chess::util::math

#elifdef _M_IA64

#    include <intrin.h> // NOLINT(build/include)

// https://learn.microsoft.com/en-gb/cpp/intrinsics/umul128
// incorrectly say that _umul128 is available for ARM

namespace chess::util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    unsigned __int64 highProduct { 0 };
    _umul128(a, b, &highProduct);
    return highProduct;
}

} // namespace chess::util::math

#else

#    include <cstdint>

#    warning "No optimized version of mul_hi64() is available, using fallback"

namespace chess::util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
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
}

} // namespace chess::util::math

#endif
