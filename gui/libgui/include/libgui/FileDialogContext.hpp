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
#include <initializer_list>
#include <nfd.hpp>
#include <print>
#include <vector>

namespace ben_bot::gui {

template <typename Func>
concept FileCallback = std::regular_invocable<Func, const std::filesystem::path&>;

// TODO: serialization for default path
struct FileDialogContext final {
    using Filter = nfdu8filteritem_t;

    FileDialogContext(
        const char*                         defaultFilename_,
        const std::initializer_list<Filter> filters_)
        : defaultFilename { defaultFilename_ }
        , filters { filters_ }
    {
    }

    const char* defaultFilename;

    std::vector<Filter> filters;

    // TODO: init to documents path?
    std::filesystem::path defaultPath;

    template <bool IsLoading>
    void show(FileCallback auto callback)
    {
        NFD::UniquePath outPath;

        const auto result = [this, &outPath]() noexcept {
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
                    std::filesystem::path { outPath.get() });

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
};

} // namespace ben_bot::gui
