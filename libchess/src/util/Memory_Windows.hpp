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

#if _WIN32_WINNT < 0x0601
#    undef _WIN32_WINNT
#    define _WIN32_WINNT 0x0601 // Force to include needed API prototypes
#endif

#include <windows.h>

namespace chess::util {

using std::size_t;

[[nodiscard]] inline void* page_aligned_alloc_win_internal([[maybe_unused]] const size_t size)
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

    auto hAdvapi32 = GetModuleHandle(TEXT("advapi32.dll"));

    if (! hAdvapi32)
        hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

    auto OpenProcessToken_f = OpenProcessToken_t((void (*)())GetProcAddress(hAdvapi32, "OpenProcessToken"));

    if (! OpenProcessToken_f)
        return nullptr;

    auto LookupPrivilegeValueA_f = LookupPrivilegeValueA_t((void (*)())GetProcAddress(hAdvapi32, "LookupPrivilegeValueA"));

    if (! LookupPrivilegeValueA_f)
        return nullptr;

    auto AdjustTokenPrivileges_f = AdjustTokenPrivileges_t((void (*)())GetProcAddress(hAdvapi32, "AdjustTokenPrivileges"));

    if (! AdjustTokenPrivileges_f)
        return nullptr;

    // We need SeLockMemoryPrivilege, so try to enable it for the process

    HANDLE hProcessToken {};

    if (! OpenProcessToken_f(
            GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcessToken)) {
        return nullptr;
    }

    LUID  luid {};
    void* mem { nullptr };

    if (LookupPrivilegeValueA_f(nullptr, "SeLockMemoryPrivilege", &luid)) {
        TOKEN_PRIVILEGES tp {};
        TOKEN_PRIVILEGES prevTp {};
        DWORD            prevTpLen = 0;

        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        // Try to enable SeLockMemoryPrivilege. Note that even if AdjustTokenPrivileges()
        // succeeds, we still need to query GetLastError() to ensure that the privileges
        // were actually obtained.

        if (AdjustTokenPrivileges_f(hProcessToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &prevTp,
                &prevTpLen)
            && GetLastError() == ERROR_SUCCESS) {
            // round up size to full pages and allocate
            size = (size + largePageSize - 1uz) & ~(largePageSize - 1uz);

            mem = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);

            // Privilege no longer needed, restore previous state
            AdjustTokenPrivileges_f(hProcessToken, FALSE, &prevTp, 0, nullptr, nullptr);
        }
    }

    CloseHandle(hProcessToken);

    return mem;
#endif
}

[[nodiscard]] inline void* page_aligned_alloc_impl(const size_t size)
{
    if (auto* mem = page_aligned_alloc_win_internal(size)) // cppcheck-suppress knownConditionTrueFalse
        return mem;

    return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

inline void page_aligned_free_impl(void* mem)
{
    if (mem != nullptr)
        VirtualFree(mem, 0, MEM_RELEASE);
}

} // namespace chess::util
