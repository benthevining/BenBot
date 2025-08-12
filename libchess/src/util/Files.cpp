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

#include <cassert>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <libchess/util/Files.hpp>
#include <memory>
#include <string>
#include <string_view>

#ifdef _WIN32
#    include "MemoryMappedFile_Windows.hpp"
#else
#    include "MemoryMappedFile_Posix.hpp"
#endif

namespace chess::util {

std::string load_file_as_string(std::filesystem::path file)
{
    file = absolute(file);

    std::ifstream input { file };

    input.exceptions(
        std::ios_base::badbit | std::ios_base::failbit);

    using Iterator = std::istreambuf_iterator<char>;

    return { Iterator { input }, Iterator {} };
}

MemoryMappedFile::MemoryMappedFile(
    const std::filesystem::path& file, const bool exclusive)
    : pimpl { std::make_unique<Pimpl>(file, exclusive) }
{
    assert(pimpl != nullptr);
}

MemoryMappedFile::~MemoryMappedFile() = default;

std::string_view MemoryMappedFile::data() const
{
    assert(pimpl != nullptr);
    return pimpl->data();
}

} // namespace chess::util
