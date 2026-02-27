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

#include <cstddef> // IWYU pragma: keep - for size_t

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#endif

#ifdef _WIN32_WINNT
#    if _WIN32_WINNT < 0x0601
#        undef _WIN32_WINNT
#    endif
#endif

#ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0601 // Force to include needed API prototypes
#endif

#include <Windows.h>

namespace chess::util::memory {

using std::size_t;

namespace impl {
    template <typename Func>
    [[nodiscard]] auto find_function(HMODULE handle, LPCSTR name) -> Func
    {
        using VoidFuncPtr = void (*)();

        return reinterpret_cast<Func>(
            reinterpret_cast<VoidFuncPtr>(GetProcAddress(handle, name)));
    }

    [[nodiscard, gnu::alloc_size(1), gnu::malloc, clang::ownership_returns(malloc)]]
    inline auto page_aligned_alloc_internal([[maybe_unused]] const size_t size) -> void*
    {
#ifndef _WIN64
        return nullptr;
#else
        using OpenProcessToken_t      = bool (*)(HANDLE, DWORD, PHANDLE);
        using LookupPrivilegeValueA_t = bool (*)(LPCSTR, LPCSTR, PLUID);
        using AdjustTokenPrivileges_t = bool (*)(HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD);

        const auto largePageSize = GetLargePageMinimum();

        if (largePageSize == 0uz)
            return nullptr;

        auto* hAdvapi32 = GetModuleHandle(TEXT("advapi32.dll"));

        if (hAdvapi32 == nullptr)
            hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

        if (hAdvapi32 == nullptr)
            return nullptr;

        const auto OpenProcessToken_f = find_function<OpenProcessToken_t>(hAdvapi32, "OpenProcessToken");

        if (OpenProcessToken_f == nullptr)
            return nullptr;

        const auto LookupPrivilegeValueA_f = find_function<LookupPrivilegeValueA_t>(hAdvapi32, "LookupPrivilegeValueA");

        if (LookupPrivilegeValueA_f == nullptr)
            return nullptr;

        const auto AdjustTokenPrivileges_f = find_function<AdjustTokenPrivileges_t>(hAdvapi32, "AdjustTokenPrivileges");

        if (AdjustTokenPrivileges_f == nullptr)
            return nullptr;

        // We need SeLockMemoryPrivilege, so try to enable it for the process

        HANDLE hProcessToken { };

        if (! OpenProcessToken_f(
                GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken)) {
            return nullptr;
        }

        LUID  luid { };
        void* mem { nullptr };

        if (LookupPrivilegeValueA_f(nullptr, "SeLockMemoryPrivilege", &luid)) {
            TOKEN_PRIVILEGES tp { }; // NOLINT(readability-identifier-length)
            TOKEN_PRIVILEGES prevTp { };
            DWORD            prevTpLen = 0;

            tp.PrivilegeCount           = 1;
            tp.Privileges[0].Luid       = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            // Try to enable SeLockMemoryPrivilege. Note that even if AdjustTokenPrivileges()
            // succeeds, we still need to query GetLastError() to ensure that the privileges
            // were actually obtained.

            if (AdjustTokenPrivileges_f(hProcessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &prevTp, &prevTpLen)
                and GetLastError() == ERROR_SUCCESS) {
                // round up size to full pages and allocate
                const auto actualSize = (size + largePageSize - 1uz) & ~(largePageSize - 1uz);

                mem = VirtualAlloc(nullptr, actualSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);

                // Privilege no longer needed, restore previous state
                AdjustTokenPrivileges_f(hProcessToken, FALSE, &prevTp, 0, nullptr, nullptr);
            }
        }

        CloseHandle(hProcessToken);

        return mem;
#endif
    }
} // namespace impl

[[nodiscard, gnu::alloc_size(1), gnu::malloc, clang::ownership_returns(malloc)]]
inline auto page_aligned_alloc_impl(const size_t size) -> void*
{
    if (auto* mem = impl::page_aligned_alloc_internal(size)) // cppcheck-suppress knownConditionTrueFalse
        return mem;

    return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

[[clang::ownership_takes(malloc, 1)]] inline void page_aligned_free_impl([[clang::noescape]] void* mem)
{
    if (mem != nullptr)
        VirtualFree(mem, 0, MEM_RELEASE);
}

} // namespace chess::util::memory
