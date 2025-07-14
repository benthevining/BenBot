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

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#endif

#include <cstddef> // IWYU pragma: keep - for size_t
#include <filesystem>
#include <libchess/util/Files.hpp>
#include <utility>
#include <windows.h>

namespace chess::util {

using std::size_t;

struct MemoryMappedFile::Pimpl final {
    Pimpl(
        const std::filesystem::path& file, const bool exclusive)
        : fileSize { static_cast<size_t>(file_size(file)) }
        , fileHandle {
            CreateFileA(
                file.string().c_str(), // NOLINT(build/include_what_you_use)
                GENERIC_READ,
                exclusive ? 0 : (FILE_SHARE_READ | FILE_SHARE_DELETE),
                nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                nullptr)
        }
    {
        if (fileHandle == INVALID_HANDLE_VALUE) {
            fileSize = 0uz;
            return;
        }

        auto* mappingHandle = CreateFileMapping(
            handle, nullptr, PAGE_READONLY,
            static_cast<DWORD>(fileSize >> 32uz),
            static_cast<DWORD>(fileSize),
            nullptr);

        if (mappingHandle != nullptr) {
            static constexpr auto zero { static_cast<DWORD>(0) };

            address = MapViewOfFile(
                mappingHandle, FILE_MAP_READ, zero, zero,
                static_cast<SIZE_T>(fileSize));

            if (address == nullptr)
                fileSize = 0uz;

            CloseHandle(mappingHandle);
        }
    }

    ~Pimpl()
    {
        if (address != nullptr)
            UnmapViewOfFile(address);

        if (fileHandle != INVALID_HANDLE_VALUE)
            CloseHandle(fileHandle);
    }

    Pimpl(const Pimpl&)            = delete;
    Pimpl& operator=(const Pimpl&) = delete;

    Pimpl(Pimpl&& other) noexcept
        : fileSize { std::exchange(other.fileSize, 0uz) }
        , fileHandle { std::exchange(other.fileHandle, INVALID_HANDLE_VALUE) }
        , address { std::exchange(other.address, nullptr) }
    {
    }

    Pimpl& operator=(Pimpl&& other) noexcept
    {
        fileSize   = std::exchange(other.fileSize, 0uz);
        fileHandle = std::exchange(other.fileHandle, INVALID_HANDLE_VALUE);
        address    = std::exchange(other.address, nullptr);

        return *this;
    }

    [[nodiscard]] std::string_view data() const
    {
        return {
            static_cast<const char*>(address),
            fileSize
        };
    }

private:
    size_t fileSize;

    HANDLE fileHandle;

    void* address { nullptr };
};

} // namespace chess::util
