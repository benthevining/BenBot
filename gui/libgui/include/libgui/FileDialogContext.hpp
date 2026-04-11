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

/** @file
    This file defines the FileDialogContext class.
    @ingroup libgui
 */

#pragma once

#include <filesystem>
#include <functional>
#include <initializer_list>
#include <nfd.hpp>
#include <vector>

namespace ben_bot::gui {

/** This struct provides methods to open native file chooser dialogs
    to load and save files.

    The default filename and filename filters are specified once at
    construction. The last viewed directory is statefully maintained
    between load/save calls.

    @note The load_file and save_file methods are blocking, modal-style
    methods.

    @ingroup libgui

    @todo make default filename stateful like the default path?
    @todo serialization for default path
 */
struct FileDialogContext final {
    using Filter   = nfdu8filteritem_t;
    using Callback = std::function<void(const std::filesystem::path&)>;

    /** Creates a file chooser dialog. */
    FileDialogContext(
        const char*                   defaultFilename_,
        std::initializer_list<Filter> filters_);

    /** Opens a file chooser dialog for file reading. */
    void load_file(const Callback& callback);

    /** Opens a file chooser dialog for file writing. */
    void save_file(const Callback& callback);

private:
    template <bool IsLoading>
    void show(const Callback& callback);

    const char* defaultFilename;

    std::vector<Filter> filters;

    std::filesystem::path defaultPath;
};

} // namespace ben_bot::gui
