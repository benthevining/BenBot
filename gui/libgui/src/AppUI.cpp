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
#include <libgui/Resources.hpp>
#include <libutil/Console.hpp>
#include <libutil/Files.hpp>
#include <nfd.hpp>
#include <nlohmann/json.hpp>
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

    state.load_from(app_state_file_path());

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
    render_board_editor(state.boardEditor, state.appSettings.showTooltips);
    render_engine_panel(state.enginePanel, state.appSettings.showTooltips);

    ImGui::Render();
}

void shutdown(const AppState& state)
{
    ImGui::DestroyContext();

    NFD::Quit();

    state.write_to(app_state_file_path());
}

using nlohmann::json;

inline constexpr string_view TAG_BOARD_EDITOR { "board_editor" };
inline constexpr string_view TAG_ENGINE { "engine" };
inline constexpr string_view TAG_SETTINGS { "app_settings" };

auto AppState::to_string() const -> std::string
{
    json data;

    data[TAG_BOARD_EDITOR] = boardEditor.to_string();
    data[TAG_ENGINE]       = enginePanel.to_string();
    data[TAG_SETTINGS]     = appSettings.to_string();

    return data.dump();
}

void AppState::update_from_string(const string_view str)
{
    const auto parsed = json::parse(str);

    boardEditor = BoardEditorState::from_string(
        parsed.at(TAG_BOARD_EDITOR).get<string_view>());

    enginePanel.update_from_string(
        parsed.at(TAG_ENGINE).get<string_view>());

    appSettings = AppSettings::from_string(
        parsed.at(TAG_SETTINGS).get<string_view>());
}

void AppState::load_from(const path& filePath)
{
    if (not exists(filePath)) {
        std::println(
            stderr, "Attempted loading state from nonexistent file: '{}'", filePath.string());
        return;
    }

    try {
        [[maybe_unused]] const auto result
            = util::files::load(filePath)
                  .transform([this](const string_view fileContent) {
                      update_from_string(fileContent);
                      return std::monostate { };
                  })
                  .transform_error(util::print_error);
    } catch (const nlohmann::detail::out_of_range& error) {
        std::println(
            stderr, "Error loading state: {}", error.what());
    }
}

void AppState::write_to(const path& filePath) const
{
    [[maybe_unused]] const auto result
        = util::files::overwrite(
            filePath, to_string())
              .transform_error(util::print_error);
}

} // namespace ben_bot::gui
