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
#include <filesystem>
#include <libchess/util/Files.hpp>

namespace chess::util {

using std::size_t;

struct MemoryMappedFile::Pimpl final {
    Pimpl(
        const std::filesystem::path& file, const bool exclusive)
        : fileSize { static_cast<size_t>(file_size(file)) }
    {
        DWORD accessMode { GENERIC_READ };
        DWORD createType { OPEN_EXISTING };
        DWORD protect { PAGE_READONLY };
        DWORD access { FILE_MAP_READ };

        auto handle = CreateFile(
            file.wstring().data(),
            accessMode,
            exclusive ? 0 : (FILE_SHARE_READ | FILE_SHARE_DELETE),
            nullptr, createType,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            nullptr);

        if (handle != INVALID_HANDLE_VALUE) {
            fileHandle = handle;

            auto* mappingHandle = CreateFileMapping(
                handle, nullptr, protect,
                static_cast<DWORD>(fileSize >> 32uz),
                static_cast<DWORD>(fileSize),
                nullptr);

            if (mappingHandle != nullptr) {
                address = MapViewOfFile(
                    mappingHandle, access,
                    static_cast<DWORD>(0uz >> 32uz),
                    static_cast<DWORD>(0uz),
                    static_cast<SIZE_T>(fileSize));

                if (address == nullptr)
                    fileSize = 0uz;

                CloseHandle(mappingHandle);
            }
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
    Pimpl(Pimpl&&)                 = delete;
    Pimpl& operator=(Pimpl&&)      = delete;

    [[nodiscard]] std::string_view data() const
    {
        return {
            static_cast<const char*>(address),
            fileSize
        };
    }

private:
    size_t fileSize;

    HANDLE fileHandle { INVALID_HANDLE_VALUE };

    void* address { nullptr };
};

} // namespace chess::util
