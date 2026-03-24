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

#include <cmath>
#include <filesystem>
#include <imgui.h>
#include <libgui/AppUI.hpp>
#include <libgui/BoardEditor.hpp>
#include <libgui/EnginePanel.hpp>
#include <libgui/Resources.hpp>
#include <libutil/Files.hpp>
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

    // TODO:
    // - Move to separate styling.cpp file?
    // - brighter color for button hover, maybe a purple?
    // - docking area hints should be paler / more transparent
    void setup_imgui_style()
    {
        // Catppuccin Mocha Palette
        static constexpr ImVec4 base { 0.117f, 0.117f, 0.172f, 1.f };     // #1e1e2e
        static constexpr ImVec4 mantle { 0.109f, 0.109f, 0.156f, 1.f };   // #181825
        static constexpr ImVec4 surface0 { 0.200f, 0.207f, 0.286f, 1.f }; // #313244
        static constexpr ImVec4 surface1 { 0.247f, 0.254f, 0.337f, 1.f }; // #3f4056
        static constexpr ImVec4 surface2 { 0.290f, 0.301f, 0.388f, 1.f }; // #4a4d63
        static constexpr ImVec4 overlay0 { 0.396f, 0.403f, 0.486f, 1.f }; // #65677c
        static constexpr ImVec4 overlay2 { 0.576f, 0.584f, 0.654f, 1.f }; // #9399b2
        static constexpr ImVec4 text { 0.803f, 0.815f, 0.878f, 1.f };     // #cdd6f4
        static constexpr ImVec4 subtext0 { 0.639f, 0.658f, 0.764f, 1.f }; // #a3a8c3
        static constexpr ImVec4 mauve { 0.796f, 0.698f, 0.972f, 1.f };    // #cba6f7
        static constexpr ImVec4 peach { 0.980f, 0.709f, 0.572f, 1.f };    // #fab387
        static constexpr ImVec4 yellow { 0.980f, 0.913f, 0.596f, 1.f };   // #f9e2af
        static constexpr ImVec4 teal { 0.580f, 0.886f, 0.819f, 1.f };     // #94e2d5
        static constexpr ImVec4 sapphire { 0.458f, 0.784f, 0.878f, 1.f }; // #74c7ec
        static constexpr ImVec4 blue { 0.533f, 0.698f, 0.976f, 1.f };     // #89b4fa
        static constexpr ImVec4 lavender { 0.709f, 0.764f, 0.980f, 1.f }; // #b4befe
        static constexpr ImVec4 transparentWhite { 0.f, 0.f, 0.f, 0.f };

        auto& style  = ImGui::GetStyle();
        auto& colors = style.Colors;

        // Main window and backgrounds
        colors[ImGuiCol_WindowBg]              = base;
        colors[ImGuiCol_ChildBg]               = base;
        colors[ImGuiCol_PopupBg]               = surface0;
        colors[ImGuiCol_Border]                = surface1;
        colors[ImGuiCol_BorderShadow]          = transparentWhite;
        colors[ImGuiCol_FrameBg]               = surface0;
        colors[ImGuiCol_FrameBgHovered]        = surface1;
        colors[ImGuiCol_FrameBgActive]         = surface2;
        colors[ImGuiCol_TitleBg]               = mantle;
        colors[ImGuiCol_TitleBgActive]         = surface0;
        colors[ImGuiCol_TitleBgCollapsed]      = mantle;
        colors[ImGuiCol_MenuBarBg]             = mantle;
        colors[ImGuiCol_ScrollbarBg]           = surface0;
        colors[ImGuiCol_ScrollbarGrab]         = surface2;
        colors[ImGuiCol_ScrollbarGrabHovered]  = overlay0;
        colors[ImGuiCol_ScrollbarGrabActive]   = overlay2;
        colors[ImGuiCol_CheckMark]             = sapphire;
        colors[ImGuiCol_SliderGrab]            = sapphire;
        colors[ImGuiCol_SliderGrabActive]      = blue;
        colors[ImGuiCol_Button]                = surface0;
        colors[ImGuiCol_ButtonHovered]         = surface1;
        colors[ImGuiCol_ButtonActive]          = surface2;
        colors[ImGuiCol_Header]                = surface0;
        colors[ImGuiCol_HeaderHovered]         = surface1;
        colors[ImGuiCol_HeaderActive]          = surface2;
        colors[ImGuiCol_Separator]             = surface1;
        colors[ImGuiCol_SeparatorHovered]      = mauve;
        colors[ImGuiCol_SeparatorActive]       = mauve;
        colors[ImGuiCol_ResizeGrip]            = surface2;
        colors[ImGuiCol_ResizeGripHovered]     = mauve;
        colors[ImGuiCol_ResizeGripActive]      = mauve;
        colors[ImGuiCol_Tab]                   = surface0;
        colors[ImGuiCol_TabHovered]            = surface2;
        colors[ImGuiCol_TabActive]             = surface1;
        colors[ImGuiCol_TabUnfocused]          = surface0;
        colors[ImGuiCol_TabUnfocusedActive]    = surface1;
        colors[ImGuiCol_DockingPreview]        = sapphire;
        colors[ImGuiCol_DockingEmptyBg]        = base;
        colors[ImGuiCol_PlotLines]             = blue;
        colors[ImGuiCol_PlotLinesHovered]      = peach;
        colors[ImGuiCol_PlotHistogram]         = teal;
        colors[ImGuiCol_PlotHistogramHovered]  = sapphire;
        colors[ImGuiCol_TableHeaderBg]         = surface0;
        colors[ImGuiCol_TableBorderStrong]     = surface1;
        colors[ImGuiCol_TableBorderLight]      = surface0;
        colors[ImGuiCol_TableRowBg]            = transparentWhite;
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4 { 1.f, 1.f, 1.f, 0.06f };
        colors[ImGuiCol_TextSelectedBg]        = surface2;
        colors[ImGuiCol_DragDropTarget]        = yellow;
        colors[ImGuiCol_NavHighlight]          = lavender;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4 { 1.f, 1.f, 1.f, 0.7f };
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4 { 0.8f, 0.8f, 0.8f, 0.2f };
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4 { 0.f, 0.f, 0.f, 0.35f };
        colors[ImGuiCol_Text]                  = text;
        colors[ImGuiCol_TextDisabled]          = subtext0;

        // Rounded corners
        style.WindowRounding    = 6.f;
        style.ChildRounding     = 6.f;
        style.FrameRounding     = 4.f;
        style.PopupRounding     = 4.f;
        style.ScrollbarRounding = 9.f;
        style.GrabRounding      = 4.f;
        style.TabRounding       = 4.f;

        // Padding and spacing
        style.WindowPadding    = ImVec2 { 8.f, 8.f };
        style.FramePadding     = ImVec2 { 5.f, 3.f };
        style.ItemSpacing      = ImVec2 { 8.f, 4.f };
        style.ItemInnerSpacing = ImVec2 { 4.f, 4.f };
        style.IndentSpacing    = 21.f;
        style.ScrollbarSize    = 14.f;
        style.GrabMinSize      = 10.f;

        // Borders
        style.WindowBorderSize = 1.f;
        style.ChildBorderSize  = 1.f;
        style.PopupBorderSize  = 1.f;
        style.FrameBorderSize  = 0.f;
        style.TabBorderSize    = 0.f;
    }
} // namespace

void initialize(const float mainScaleFactor, AppState& state)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    if (not exists(imgui_ini_path())) {
        const auto defaultData = resources::get_default_imgui_ini_data();
        ImGui::LoadIniSettingsFromMemory(defaultData.data(), defaultData.size());
    }

    if (const auto filePath = app_state_file_path();
        exists(filePath)) {
        [[maybe_unused]] const auto result
            = util::files::load(filePath)
                  .transform([&state](const string_view fileContent) {
                      state.update_from_string(fileContent);
                      return std::monostate { };
                  });
    }

    { // Setup scaling
        auto& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScaleFactor);
        style.FontScaleDpi = mainScaleFactor;
    }

    setup_imgui_style();

    // TODO: load custom fonts
}

void render(AppState& state)
{
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    render_board_editor(state.boardEditor);

    render_engine_panel(state.enginePanel);

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

auto AppState::to_string() const -> std::string
{
    json data;

    data[TAG_BOARD_EDITOR] = boardEditor.to_string();
    data[TAG_ENGINE]       = enginePanel.to_string();

    return data.dump();
}

void AppState::update_from_string(const string_view str)
{
    const auto parsed = json::parse(str);

    boardEditor = BoardEditorState::from_string(
        parsed.at(TAG_BOARD_EDITOR).get<string_view>());

    enginePanel.update_from_string(
        parsed.at(TAG_ENGINE).get<string_view>());
}

} // namespace ben_bot::gui
