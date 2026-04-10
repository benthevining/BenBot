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

// TODO: default path
template <bool IsLoading>
void show(
    [[maybe_unused]] const char*  defaultFilename,
    const std::span<const Filter> filters,
    Callback auto                 callback)
{
    NFD::UniquePath outPath;

    const auto result = [&outPath, defaultFilename, filters]() noexcept {
        if constexpr (IsLoading) {
            return OpenDialog(
                outPath,
                filters.data(), filters.size());
        } else {
            return SaveDialog(
                outPath,
                filters.data(), filters.size(),
                nullptr, defaultFilename);
        }
    }();

    switch (result) {
        case NFD_OKAY:
            callback(
                path { outPath.get() });
            break;

        case NFD_CANCEL:
            std::println("Info: user canceled file selection dialog");
            break;

        case NFD_ERROR:
            std::println(
                stderr, "Info: error with file selection dialog");
            break;

        default: break;
    }
}

} // namespace ben_bot::gui::file_dialog
