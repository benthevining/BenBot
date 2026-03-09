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

#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iterator>
#include <libutil/Files.hpp>
#include <string>
#include <string_view>

namespace util {

using std::string;

auto load_file_as_string(
    const std::filesystem::path& file)
    -> std::expected<string, string>
{
    std::ifstream input { absolute(file) };

    if (not input.is_open()) {
        return std::unexpected { std::format(
            "Could not open file for reading at path '{}'",
            file.string()) };
    }

    try {
        input.exceptions(
            std::ios_base::badbit | std::ios_base::failbit);

        using Iterator = std::istreambuf_iterator<char>;

        return string { Iterator { input }, Iterator { } };
    } catch (const std::exception& exception) {
        return std::unexpected { std::format(
            "Error while reading file at path '{}': {}",
            file.string(), exception.what()) };
    }
}

auto overwrite_file(
    const std::filesystem::path& file, const std::string_view text)
    -> std::expected<void, std::string>
{
    std::ofstream output { absolute(file) };

    if (not output.is_open()) {
        return std::unexpected { std::format(
            "Could not open file for writing at path '{}'",
            file.string()) };
    }

    try {
        output.exceptions(
            std::ios_base::badbit | std::ios_base::failbit);

        output << text;

        return { };
    } catch (const std::exception& exception) {
        return std::unexpected {
            std::format(
                "Error while writing file at path '{}': {}",
                file.string(), exception.what())
        };
    }
}

} // namespace util
