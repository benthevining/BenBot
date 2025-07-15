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
#include <fcntl.h>
#include <filesystem>
#include <libchess/util/Files.hpp>
#include <string_view>
#include <sys/mman.h>
#include <sys/types.h> // IWYU pragma: keep - for off_t
#include <unistd.h>
#include <utility>

namespace chess::util {

using std::size_t;

struct MemoryMappedFile::Pimpl final {
    Pimpl(
        const std::filesystem::path& file, const bool exclusive)
        : fileSize { static_cast<size_t>(file_size(file)) }
        , fileHandle { open(file.c_str(), O_RDONLY) }
    {
        if (fileHandle != -1) {
            auto* ptr = mmap(
                nullptr, fileSize, PROT_READ,
                exclusive ? MAP_PRIVATE : MAP_SHARED,
                fileHandle, static_cast<off_t>(0));

            if (ptr != MAP_FAILED) {
                address = ptr;
                madvise(ptr, fileSize, MADV_SEQUENTIAL);
            } else {
                fileSize = 0uz;
            }

            close(fileHandle);
            fileHandle = 0;
        }
    }

    ~Pimpl()
    {
        if (address != nullptr)
            munmap(address, fileSize);

        if (fileHandle != 0)
            close(fileHandle);
    }

    Pimpl(const Pimpl&)            = delete;
    Pimpl& operator=(const Pimpl&) = delete;

    Pimpl(Pimpl&& other) noexcept
        : fileSize { std::exchange(other.fileSize, 0uz) }
        , fileHandle { std::exchange(other.fileHandle, 0) }
        , address { std::exchange(other.address, nullptr) }
    {
    }

    Pimpl& operator=(Pimpl&& other) noexcept
    {
        fileSize   = std::exchange(other.fileSize, 0uz);
        fileHandle = std::exchange(other.fileHandle, 0);
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

    int fileHandle;

    void* address { nullptr };
};

} // namespace chess::util
