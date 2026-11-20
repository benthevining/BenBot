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
    This file defines the application settings panel state and rendering function.
    @ingroup libgui
 */

#pragma once

#include <libgui/ErrorPopup.hpp>
#include <libgui/FileDialogContext.hpp>
#include <string>
#include <string_view>

namespace ben_bot::gui {

struct AppState;

/** This struct encapsulates the state of the application settings panel.
    @ingroup libgui
 */
struct AppSettings final {
    /** Controls whether tooltips are shown. */
    bool showTooltips { true };

    /** File dialog context for loading and saving the current application
        state to/from JSON files.
     */
    FileDialogContext stateLoadSave {
        "AppState.json",
        { { "JSON", "json" } }
    };

    /** Error popup context used for writing the current application state
        to the resource files in the source tree.
     */
    [[maybe_unused]] ErrorPopup defaultStateSaveError { "Default state save error" };

    /** Serializes this state to a JSON string. */
    [[nodiscard]] auto to_string() const -> std::string;

    /** Restores a saved state from a JSON string. */
    [[nodiscard]] static auto from_string(std::string_view str) -> AppSettings;
};

/** Renders an application settings panel.

    @ingroup libgui
    @relates AppSettings
 */
void render_app_settings_panel(
    AppState& state);

} // namespace ben_bot::gui
