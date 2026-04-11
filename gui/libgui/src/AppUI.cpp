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

#include "ImUtil.hpp" // NOLINT(build/include_subdir)
#include <cmath>
#include <filesystem>
#include <imgui.h>
#include <libgui/AppSettingsPanel.hpp>
#include <libgui/AppUI.hpp>
#include <libgui/BoardEditor.hpp>
#include <libgui/EnginePanel.hpp>
#include <libgui/GameViewer.hpp>
#include <libgui/PSTEditor.hpp>
#include <libgui/Resources.hpp>
#include <libutil/Console.hpp>
#include <libutil/Files.hpp>
#include <nfd.hpp>
#include <nlohmann/json.hpp>
#include <print>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::gui {

auto get_scaled_default_dimensions(const float scaleFactor)
    -> std::pair<int, int>
{
    static constexpr auto DefaultWidth  = 1280.f;
    static constexpr auto DefaultHeight = 800.f;

    return std::make_pair(
        static_cast<int>(std::round(DefaultWidth * scaleFactor)),
        static_cast<int>(std::round(DefaultHeight * scaleFactor)));
}

using std::filesystem::path;
using std::string_view;

auto get_default_imgui_ini_path() -> path
{
    return "imgui.ini";
}

auto get_default_app_state_path() -> path
{
    return "benbot_state.json";
}

void load_default_ui_layout()
{
    const auto defaultData = resources::get_default_imgui_ini_data();

    ImGui::LoadIniSettingsFromMemory(
        defaultData.data(), defaultData.size());
}

void initialize(
    const float mainScaleFactor, AppState& state)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    {
        auto& io_ = ImGui::GetIO();
        io_.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io_.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io_.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    if (not exists(get_default_imgui_ini_path()))
        load_default_ui_layout();

    if (const auto stateFile = get_default_app_state_path();
        exists(stateFile)) {
        state.load_from(stateFile);
    } else {
        state.update_from_string(
            resources::get_default_app_state());
    }

    state.enginePanel.engine.handle_command("ucinewgame");

    { // Setup scaling
        auto& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScaleFactor);
        style.FontScaleDpi = mainScaleFactor;
        style.FontSizeBase = 20.f;
    }

    setup_imgui_style();

    NFD::Init();
}

void render(AppState& state)
{
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    render_app_settings_panel(state);

    const bool showTooltips = state.appSettings.showTooltips;

    render_board_editor(
        state.boardEditor, showTooltips, state.enginePanel.engine);

    render_engine_panel(
        state.enginePanel, showTooltips);

    render_game_viewer(
        state.gameViewer, state.enginePanel.engine, state.boardEditor.position.position, showTooltips);

    render_pst_editor(
        state.pstEditor, state.enginePanel.engine, showTooltips);

    ImGui::Render();
}

void shutdown(const AppState& state)
{
    ImGui::DestroyContext();

    NFD::Quit();

    state.write_to(
        get_default_app_state_path());
}

using nlohmann::json;

inline constexpr string_view TAG_BOARD_EDITOR { "board_editor" };
inline constexpr string_view TAG_ENGINE { "engine" };
inline constexpr string_view TAG_GAME_VIEWER { "game_viewer" };
inline constexpr string_view TAG_SETTINGS { "app_settings" };

auto AppState::to_string() const -> std::string
{
    json data;

    data[TAG_BOARD_EDITOR] = boardEditor.to_string();
    data[TAG_ENGINE]       = enginePanel.to_string();
    data[TAG_SETTINGS]     = appSettings.to_string();
    data[TAG_GAME_VIEWER]  = gameViewer.to_string();

    return data.dump();
}

void AppState::update_from_string(const string_view str)
try {
    const auto parsed = json::parse(str);

    boardEditor = BoardEditorState::from_string(
        parsed.at(TAG_BOARD_EDITOR).get<string_view>());

    enginePanel.update_from_string(
        parsed.at(TAG_ENGINE).get<string_view>());

    appSettings = AppSettings::from_string(
        parsed.at(TAG_SETTINGS).get<string_view>());

    gameViewer = GameViewerState::from_string(
        parsed.at(TAG_GAME_VIEWER).get<string_view>());
} catch (const nlohmann::detail::out_of_range& error) {
    std::println(
        stderr, "Error loading state: {}", error.what());
}

void AppState::load_from(const path& filePath)
{
    if (not exists(filePath)) {
        std::println(
            stderr, "Attempted loading state from nonexistent file: '{}'", filePath.string());
        return;
    }

    [[maybe_unused]] const auto result
        = util::files::load(filePath)
              .transform([this](const string_view fileContent) {
                  update_from_string(fileContent);
                  return std::monostate { };
              })
              .transform_error(util::print_error);
}

void AppState::write_to(const path& filePath) const
{
    [[maybe_unused]] const auto result
        = util::files::overwrite(
            filePath, to_string())
              .transform_error(util::print_error);
}

} // namespace ben_bot::gui
