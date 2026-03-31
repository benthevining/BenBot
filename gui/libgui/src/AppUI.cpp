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
#include <cstdio>
#include <filesystem>
#include <imgui.h>
#include <libgui/AppUI.hpp>
#include <libgui/BoardEditor.hpp>
#include <libgui/EnginePanel.hpp>
#include <libgui/Resources.hpp>
#include <libutil/Files.hpp>
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

namespace {
    using std::filesystem::path;
    using std::string_view;

    [[nodiscard]] auto imgui_ini_path() -> path
    {
        return "imgui.ini";
    }

    [[nodiscard]] auto app_state_file_path() -> path
    {
        return "benbot_state.json";
    }

    void render_app_settings(bool& showTooltips)
    {
        if (ImGui::Begin("App settings")) {
            ImGui::Checkbox("Show tooltips", &showTooltips);

            if (showTooltips)
                ImGui::SetItemTooltip("Display widget tooltips");
        }

        ImGui::End();
    }
} // namespace

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

    if (not exists(imgui_ini_path())) {
        const auto defaultData = resources::get_default_imgui_ini_data();
        ImGui::LoadIniSettingsFromMemory(defaultData.data(), defaultData.size());
    }

    if (const auto filePath = app_state_file_path();
        exists(filePath)) {
        try {
            [[maybe_unused]] const auto result
                = util::files::load(filePath)
                      .transform([&state](const string_view fileContent) {
                          state.update_from_string(fileContent);
                          return std::monostate { };
                      });
        } catch (const nlohmann::detail::out_of_range& error) {
            std::println(
                stderr, "Error loading state: {}", error.what());
        }
    }

    { // Setup scaling
        auto& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScaleFactor);
        style.FontScaleDpi = mainScaleFactor;
        style.FontSizeBase = 20.f;
    }

    setup_imgui_style();
}

void render(AppState& state)
{
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    render_app_settings(state.showTooltips);
    render_board_editor(state.boardEditor, state.showTooltips);
    render_engine_panel(state.enginePanel, state.showTooltips);

    ImGui::Render();
}

void shutdown(const AppState& state)
{
    ImGui::DestroyContext();

    [[maybe_unused]] const auto result
        = util::files::overwrite(
            app_state_file_path(),
            state.to_string());
}

using nlohmann::json;

inline constexpr string_view TAG_BOARD_EDITOR { "board_editor" };
inline constexpr string_view TAG_ENGINE { "engine" };
inline constexpr string_view TAG_TOOLTIPS { "show_tooltips" };

auto AppState::to_string() const -> std::string
{
    json data;

    data[TAG_BOARD_EDITOR] = boardEditor.to_string();
    data[TAG_ENGINE]       = enginePanel.to_string();
    data[TAG_TOOLTIPS]     = showTooltips;

    return data.dump();
}

void AppState::update_from_string(const string_view str)
{
    const auto parsed = json::parse(str);

    boardEditor = BoardEditorState::from_string(
        parsed.at(TAG_BOARD_EDITOR).get<string_view>());

    enginePanel.update_from_string(
        parsed.at(TAG_ENGINE).get<string_view>());

    showTooltips = parsed.at(TAG_TOOLTIPS).get<bool>();
}

} // namespace ben_bot::gui
