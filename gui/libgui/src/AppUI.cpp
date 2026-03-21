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
    // - coalesce rounded corners flags, etc. Move to separate styling.cpp file?
    // - brighter color for button hover, maybe a purple?
    // - get rid of ugly green used for checkmarks
    // - docking area hints should be paler / more transparent
    void setup_catppuccin_mocha_theme()
    {
        ImGuiStyle& style  = ImGui::GetStyle();
        ImVec4*     colors = style.Colors;

        // Catppuccin Mocha Palette
        // --------------------------------------------------------
        const ImVec4 base     = ImVec4(0.117f, 0.117f, 0.172f, 1.0f); // #1e1e2e
        const ImVec4 mantle   = ImVec4(0.109f, 0.109f, 0.156f, 1.0f); // #181825
        const ImVec4 surface0 = ImVec4(0.200f, 0.207f, 0.286f, 1.0f); // #313244
        const ImVec4 surface1 = ImVec4(0.247f, 0.254f, 0.337f, 1.0f); // #3f4056
        const ImVec4 surface2 = ImVec4(0.290f, 0.301f, 0.388f, 1.0f); // #4a4d63
        const ImVec4 overlay0 = ImVec4(0.396f, 0.403f, 0.486f, 1.0f); // #65677c
        const ImVec4 overlay2 = ImVec4(0.576f, 0.584f, 0.654f, 1.0f); // #9399b2
        const ImVec4 text     = ImVec4(0.803f, 0.815f, 0.878f, 1.0f); // #cdd6f4
        const ImVec4 subtext0 = ImVec4(0.639f, 0.658f, 0.764f, 1.0f); // #a3a8c3
        const ImVec4 mauve    = ImVec4(0.796f, 0.698f, 0.972f, 1.0f); // #cba6f7
        const ImVec4 peach    = ImVec4(0.980f, 0.709f, 0.572f, 1.0f); // #fab387
        const ImVec4 yellow   = ImVec4(0.980f, 0.913f, 0.596f, 1.0f); // #f9e2af
        const ImVec4 green    = ImVec4(0.650f, 0.890f, 0.631f, 1.0f); // #a6e3a1
        const ImVec4 teal     = ImVec4(0.580f, 0.886f, 0.819f, 1.0f); // #94e2d5
        const ImVec4 sapphire = ImVec4(0.458f, 0.784f, 0.878f, 1.0f); // #74c7ec
        const ImVec4 blue     = ImVec4(0.533f, 0.698f, 0.976f, 1.0f); // #89b4fa
        const ImVec4 lavender = ImVec4(0.709f, 0.764f, 0.980f, 1.0f); // #b4befe

        // Main window and backgrounds
        colors[ImGuiCol_WindowBg]              = base;
        colors[ImGuiCol_ChildBg]               = base;
        colors[ImGuiCol_PopupBg]               = surface0;
        colors[ImGuiCol_Border]                = surface1;
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
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
        colors[ImGuiCol_CheckMark]             = green;
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
        colors[ImGuiCol_PlotHistogramHovered]  = green;
        colors[ImGuiCol_TableHeaderBg]         = surface0;
        colors[ImGuiCol_TableBorderStrong]     = surface1;
        colors[ImGuiCol_TableBorderLight]      = surface0;
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
        colors[ImGuiCol_TextSelectedBg]        = surface2;
        colors[ImGuiCol_DragDropTarget]        = yellow;
        colors[ImGuiCol_NavHighlight]          = lavender;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);
        colors[ImGuiCol_Text]                  = text;
        colors[ImGuiCol_TextDisabled]          = subtext0;

        // Rounded corners
        style.WindowRounding    = 6.0f;
        style.ChildRounding     = 6.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;

        // Padding and spacing
        style.WindowPadding    = ImVec2(8.0f, 8.0f);
        style.FramePadding     = ImVec2(5.0f, 3.0f);
        style.ItemSpacing      = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.IndentSpacing    = 21.0f;
        style.ScrollbarSize    = 14.0f;
        style.GrabMinSize      = 10.0f;

        // Borders
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize  = 1.0f;
        style.PopupBorderSize  = 1.0f;
        style.FrameBorderSize  = 0.0f;
        style.TabBorderSize    = 0.0f;
    }

    void style_imgui_colors()
    {
        setup_catppuccin_mocha_theme();
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

        style.GrabRounding     = 4.0f;
        style.WindowRounding   = 5.0f;
        style.FrameRounding    = 4.0f;
        style.FrameBorderSize  = 0.0f;
        style.PopupBorderSize  = 1.0f;
        style.WindowBorderSize = 0.0f;
        style.PopupBorderSize  = 0.0f;
        style.ChildBorderSize  = 1.0f;
        style.WindowMinSize    = { 200.0f, 200.0f };
    }

    style_imgui_colors();

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // style.FontSizeBase = 20.0f;
    // io.Fonts->AddFontDefaultVector();
    // io.Fonts->AddFontDefaultBitmap();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    // IM_ASSERT(font != nullptr);
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

auto AppState::to_string() const -> std::string
{
    json data;

    data[TAG_BOARD_EDITOR] = boardEditor.to_string();

    return data.dump();
}

void AppState::update_from_string(const string_view str)
{
    const auto parsed = json::parse(str);

    boardEditor = BoardEditorState::from_string(
        parsed.at(TAG_BOARD_EDITOR).get<string_view>());
}

} // namespace ben_bot::gui
