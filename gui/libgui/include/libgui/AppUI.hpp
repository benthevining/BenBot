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
    This is the main include for ``libgui``. This file defines the main
    application state and the primary initialize/render/shutdown API.
    @ingroup libgui
 */

#pragma once

#include <filesystem>
#include <libgui/AppSettingsPanel.hpp>
#include <libgui/BoardEditor.hpp>
#include <libgui/EnginePanel.hpp>
#include <libgui/GameViewer.hpp>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::gui {

/** The application name.
    @ingroup libgui
 */
inline constexpr auto AppName { "BenBot GUI" };

/** Returns the default window dimensions, scaled by the given factor.
    @ingroup libgui
 */
[[nodiscard]] auto get_scaled_default_dimensions(float scaleFactor)
    -> std::pair<int, int>;

/** Returns the path to the local ``imgui.ini`` file being saved to
    at each app exit and reloaded each relaunch. Typically found next
    to the executable itself.

    @ingroup libgui
 */
[[nodiscard]] auto get_default_imgui_ini_path() -> std::filesystem::path;

/** Returns the path to the local JSON file being saved to at each app exit
    and reloaded each relaunch with the entire application state. Typically
    found next to the executable itself.

    @ingroup libgui
 */
[[nodiscard]] auto get_default_app_state_path() -> std::filesystem::path;

/** Loads the default UI layout from the embedded resource.
    @ingroup libgui
 */
void load_default_ui_layout();

/** The top-level state of the application.
    @ingroup libgui
 */
struct AppState final {
    /** The state of the application settings panel. */
    AppSettings appSettings;

    /** The state of the board editor. */
    BoardEditorState boardEditor;

    /** The state of the engine panel. */
    EnginePanelState enginePanel;

    /** The state of the PGN game viewer. */
    GameViewerState gameViewer;

    /** Serializes this state to a JSON string. */
    [[nodiscard]] auto to_string() const -> std::string;

    /** Restores the state from a saved JSON string. */
    void update_from_string(std::string_view str);

    /** Loads a saved state file. */
    void load_from(const std::filesystem::path& filePath);

    /** Writes the current state to a file. */
    void write_to(const std::filesystem::path& filePath) const;
};

/** Initializes the application.
    This should be called once at startup.

    @ingroup libgui
    @relates AppState
 */
void initialize(
    float mainScaleFactor, AppState& state);

/** Renders the application.
    This should be called in the main event loop.

    @ingroup libgui
    @relates AppState
 */
void render(AppState& state);

/** Shuts down the application.
    This should be called after the main event loop has exited.

    @ingroup libgui
    @relates AppState
 */
void shutdown(const AppState& state);

} // namespace ben_bot::gui
