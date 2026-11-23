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

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <libutil/Threading.hpp>

inline constexpr auto TAGS { "[util][threading]" };

using std::size_t;

namespace {

inline constexpr auto FRAME_SIZE { 4uz * 1024uz };

// uses at least FRAME_SIZE bytes of stack per call. The buffer is accessed through a volatile
// pointer so that the compiler can't optimize it away or turn the recursion into a loop
[[gnu::noinline]] auto use_stack(const size_t depth) -> size_t
{
    std::array<char, FRAME_SIZE> buffer { };

    volatile char* const data = buffer.data();

    const auto idx = depth % FRAME_SIZE;

    data[idx] = 1;

    if (depth == 0uz)
        return static_cast<size_t>(data[idx]);

    return use_stack(depth - 1uz) + static_cast<size_t>(data[idx]);
}

} // namespace

TEST_CASE("LargeStackThread - runs its function", TAGS)
{
    bool ran { false };

    util::LargeStackThread thread { 1024uz * 1024uz, [&ran] { ran = true; } };

    thread.join();

    REQUIRE(ran);
}

TEST_CASE("LargeStackThread - has a stack bigger than the platform default", TAGS)
{
    // 4 MiB of recursion overflows the default stack of a new thread on macOS (512 KiB)
    // and Windows (1 MiB)
    static constexpr auto DEPTH { 1024uz };

    size_t result { 0uz };

    util::LargeStackThread thread { 16uz * 1024uz * 1024uz, [&result] { result = use_stack(DEPTH); } };

    thread.join();

    REQUIRE(result == DEPTH + 1uz);
}
