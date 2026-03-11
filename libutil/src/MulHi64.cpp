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

#include <cstdint>
#include <libutil/Math.hpp>

namespace {
[[nodiscard, gnu::const, maybe_unused]] auto mul_hi64_fallback(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    auto get_lo_32_bits = [](const uint64_t value) noexcept {
        return static_cast<uint64_t>(static_cast<std::uint32_t>(value));
    };

    auto get_hi_32_bits = [](const uint64_t value) noexcept {
        return static_cast<uint64_t>(value >> UINT64_C(32));
    };

    const auto a_lo = get_lo_32_bits(first);
    const auto a_hi = get_hi_32_bits(first);

    const auto b_lo = get_lo_32_bits(second);
    const auto b_hi = get_hi_32_bits(second);

    const auto c_1 = get_hi_32_bits(a_lo * b_lo);
    const auto c_2 = (a_hi * b_lo) + c_1;
    const auto c_3 = (a_lo * b_hi) + get_lo_32_bits(c_2);

    return (a_hi * b_hi) + get_hi_32_bits(c_2) + get_hi_32_bits(c_3);
}
} // namespace

#ifdef __SIZEOF_INT128__

namespace util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    __extension__ using uint128_t = unsigned __int128;

    const uint128_t prod = static_cast<uint128_t>(first) * static_cast<uint128_t>(second);

    return static_cast<uint64_t>(
        prod >> static_cast<uint128_t>(64));
}

} // namespace util::math

#elif defined(_M_X64) or defined(_M_ARM64) // MSVC for x86-64 or AArch64

#    include <intrin.h>

namespace util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    return __umulh(first, second);
}

} // namespace util::math

#elifdef _M_IA64

#    include <intrin.h> // NOLINT(build/include)

// https://learn.microsoft.com/en-gb/cpp/intrinsics/umul128
// incorrectly say that _umul128 is available for ARM

namespace util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    unsigned __int64 highProduct { 0 };
    _umul128(a, b, &highProduct);
    return highProduct;
}

} // namespace util::math

#else
#    warning "No optimized version of mul_hi64() is available, using fallback"

namespace util::math {

auto mul_hi64(
    const uint64_t first, const uint64_t second) noexcept
    -> uint64_t
{
    return mul_hi64_fallback(first, second);
}

} // namespace util::math

#endif
