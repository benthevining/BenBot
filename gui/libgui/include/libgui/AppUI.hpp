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

#include <filesystem>
#include <libgui/AppSettingsPanel.hpp>
#include <libgui/BoardEditor.hpp>
#include <libgui/EnginePanel.hpp>
#include <libgui/GameViewer.hpp>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::gui {

inline constexpr auto AppName { "BenBot GUI" };

[[nodiscard]] auto get_scaled_default_dimensions(float scaleFactor)
    -> std::pair<int, int>;

[[nodiscard]] auto get_default_imgui_ini_path() -> std::filesystem::path;
[[nodiscard]] auto get_default_app_state_path() -> std::filesystem::path;

void load_default_ui_layout();

struct AppState final {
    AppSettings appSettings;

    BoardEditorState boardEditor;

    EnginePanelState enginePanel;

    GameViewerState gameViewer;

    [[nodiscard]] auto to_string() const -> std::string;

    void update_from_string(std::string_view str);

    void load_from(const std::filesystem::path& filePath);

    void write_to(const std::filesystem::path& filePath) const;
};

void initialize(
    float mainScaleFactor, AppState& state);

void render(AppState& state);

void shutdown(const AppState& state);

} // namespace ben_bot::gui
