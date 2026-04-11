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

#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <libgui/FileDialogContext.hpp>
#include <nfd.hpp>
#include <print>

namespace ben_bot::gui {

FileDialogContext::FileDialogContext(
    const char*                         defaultFilename_,
    const std::initializer_list<Filter> filters_)
    : defaultFilename { defaultFilename_ }
    , filters { filters_ }
{
}

void FileDialogContext::load_file(const Callback& callback)
{
    this->show<true>(callback);
}

void FileDialogContext::save_file(const Callback& callback)
{
    this->show<false>(callback);
}

template <bool IsLoading>
void FileDialogContext::show(const Callback& callback)
{
    NFD::UniquePath outPath;

    const auto result = [this, &outPath]() noexcept {
        if constexpr (IsLoading) {
            return OpenDialog(
                outPath,
                filters.data(), static_cast<nfdfiltersize_t>(filters.size()),
                defaultPath.c_str());
        } else {
            return SaveDialog(
                outPath,
                filters.data(), static_cast<nfdfiltersize_t>(filters.size()),
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

        default: [[fallthrough]];
        case NFD_ERROR:
            std::println(
                stderr, "Info: error with file selection dialog");
    }
}

template void FileDialogContext::show<true>(const Callback&);
template void FileDialogContext::show<false>(const Callback&);

} // namespace ben_bot::gui
