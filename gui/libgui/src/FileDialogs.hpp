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

#include <concepts>
#include <cstdio>
#include <filesystem>
#include <nfd.hpp>
#include <print>
#include <span>

namespace ben_bot::gui::file_dialog {

using std::filesystem::path;
using Filter = nfdu8filteritem_t;

template <typename Func>
concept Callback = std::regular_invocable<Func, const path&>;

template <bool IsLoading>
void show(
    [[maybe_unused]] const char*  defaultFilename,
    const std::span<const Filter> filters,
    path&                         defaultPath,
    Callback auto                 callback)
{
    NFD::UniquePath outPath;

    const auto result = [&outPath, defaultFilename, filters, &defaultPath]() noexcept {
        if constexpr (IsLoading) {
            return OpenDialog(
                outPath,
                filters.data(), filters.size(),
                defaultPath.c_str());
        } else {
            return SaveDialog(
                outPath,
                filters.data(), filters.size(),
                defaultPath.c_str(),
                defaultFilename);
        }
    }();

    switch (result) {
        case NFD_OKAY: {
            const auto resultPath = absolute(
                path { outPath.get() });

            defaultPath = resultPath.parent_path();

            callback(resultPath);
            return;
        }

        case NFD_CANCEL:
            std::println("Info: user canceled file selection dialog");
            return;

        case NFD_ERROR:
            std::println(
                stderr, "Info: error with file selection dialog");
            return;

        default: return;
    }
}

} // namespace ben_bot::gui::file_dialog
