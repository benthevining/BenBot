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

    // V3 theme v1.1
    // - rlyeh, public domain
    void igThemeV3(int hue07, int alt07, int nav07)
    {
        auto& style = ImGui::GetStyle();

        style.Alpha         = 1.0f;
        style.DisabledAlpha = 0.3f;

        style.WindowPadding    = ImVec2(4, 8.f);
        style.FramePadding     = ImVec2(4, 4.f);
        style.ItemSpacing      = ImVec2(8.f, 2.f);
        style.ItemInnerSpacing = ImVec2(4, 4);
        style.IndentSpacing    = 16;
        style.ScrollbarSize    = 18;
        style.GrabMinSize      = 20;

        style.WindowBorderSize = 1;
        style.ChildBorderSize  = 1;
        style.PopupBorderSize  = 1;
        style.FrameBorderSize  = 0;

        style.WindowRounding    = 4;
        style.ChildRounding     = 6;
        style.FrameRounding     = 12;
        style.PopupRounding     = 4;
        style.ScrollbarRounding = 12.f;
        style.GrabRounding      = style.FrameRounding;

        style.TabBorderSize                    = 0;
        style.TabBarBorderSize                 = 2;
        style.TabBarOverlineSize               = 2;
        style.TabCloseButtonMinWidthSelected   = -1;
        style.TabCloseButtonMinWidthUnselected = -1;
        style.TabRounding                      = 1.f;

        style.CellPadding = ImVec2(8.0f, 4.0f);

        style.WindowTitleAlign         = ImVec2(0.5f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Right;

        style.ColorButtonPosition     = ImGuiDir_Right;
        style.ButtonTextAlign         = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign     = ImVec2(0.5f, 0.5f);
        style.SeparatorTextAlign.x    = 1.00f;
        style.SeparatorTextBorderSize = 1;
        style.SeparatorTextPadding    = ImVec2(0, 0);

        style.WindowMinSize     = ImVec2(32.0f, 16.0f);
        style.ColumnsMinSpacing = 6.0f;

        style.CircleTessellationMaxError = 0.3f;

        auto dim = [&](ImVec4 hi) {
            float h, s, v;
            ImGui::ColorConvertRGBtoHSV(hi.x, hi.y, hi.z, h, s, v);
            ImVec4 dim = ImColor::HSV(h, s, v * 0.65, hi.w).Value;
            if (hi.z > hi.x && hi.z > hi.y)
                return ImVec4(dim.x, dim.y, hi.z, dim.w);
            return dim;
        };

        const ImVec4 cyan    = ImVec4(000 / 255.f, 192 / 255.f, 255 / 255.f, 1.00f);
        const ImVec4 red     = ImVec4(230 / 255.f, 000 / 255.f, 000 / 255.f, 1.00f);
        const ImVec4 yellow  = ImVec4(240 / 255.f, 210 / 255.f, 000 / 255.f, 1.00f);
        const ImVec4 orange  = ImVec4(255 / 255.f, 144 / 255.f, 000 / 255.f, 1.00f);
        const ImVec4 lime    = ImVec4(192 / 255.f, 255 / 255.f, 000 / 255.f, 1.00f);
        const ImVec4 aqua    = ImVec4(000 / 255.f, 255 / 255.f, 192 / 255.f, 1.00f);
        const ImVec4 magenta = ImVec4(255 / 255.f, 000 / 255.f, 88 / 255.f, 1.00f);
        const ImVec4 purple  = ImVec4(192 / 255.f, 000 / 255.f, 255 / 255.f, 1.00f);

        ImVec4 alt = cyan;
        /**/ if (alt07 == 0 || alt07 == 'C')
            alt = cyan;
        else if (alt07 == 1 || alt07 == 'R')
            alt = red;
        else if (alt07 == 2 || alt07 == 'Y')
            alt = yellow;
        else if (alt07 == 3 || alt07 == 'O')
            alt = orange;
        else if (alt07 == 4 || alt07 == 'L')
            alt = lime;
        else if (alt07 == 5 || alt07 == 'A')
            alt = aqua;
        else if (alt07 == 6 || alt07 == 'M')
            alt = magenta;
        else if (alt07 == 7 || alt07 == 'P')
            alt = purple;

        ImVec4 hi = cyan, lo = dim(cyan);
        /**/ if (hue07 == 0 || hue07 == 'C')
            lo = dim(hi = cyan);
        else if (hue07 == 1 || hue07 == 'R')
            lo = dim(hi = red);
        else if (hue07 == 2 || hue07 == 'Y')
            lo = dim(hi = yellow);
        else if (hue07 == 3 || hue07 == 'O')
            lo = dim(hi = orange);
        else if (hue07 == 4 || hue07 == 'L')
            lo = dim(hi = lime);
        else if (hue07 == 5 || hue07 == 'A')
            lo = dim(hi = aqua);
        else if (hue07 == 6 || hue07 == 'M')
            lo = dim(hi = magenta);
        else if (hue07 == 7 || hue07 == 'P')
            lo = dim(hi = purple);

        ImVec4 nav = orange;
        /**/ if (nav07 == 0 || nav07 == 'C')
            nav = cyan;
        else if (nav07 == 1 || nav07 == 'R')
            nav = red;
        else if (nav07 == 2 || nav07 == 'Y')
            nav = yellow;
        else if (nav07 == 3 || nav07 == 'O')
            nav = orange;
        else if (nav07 == 4 || nav07 == 'L')
            nav = lime;
        else if (nav07 == 5 || nav07 == 'A')
            nav = aqua;
        else if (nav07 == 6 || nav07 == 'M')
            nav = magenta;
        else if (nav07 == 7 || nav07 == 'P')
            nav = purple;

        const ImVec4
            link  = ImVec4(0.26f, 0.59f, 0.98f, 1.00f),
            grey0 = ImVec4(0.04f, 0.05f, 0.07f, 1.00f),
            grey1 = ImVec4(0.08f, 0.09f, 0.11f, 1.00f),
            grey2 = ImVec4(0.10f, 0.11f, 0.13f, 1.00f),
            grey3 = ImVec4(0.12f, 0.13f, 0.15f, 1.00f),
            grey4 = ImVec4(0.16f, 0.17f, 0.19f, 1.00f),
            grey5 = ImVec4(0.18f, 0.19f, 0.21f, 1.00f);

#define Luma(v, a) ImVec4((v) / 100.f, (v) / 100.f, (v) / 100.f, (a) / 100.f)

        style.Colors[ImGuiCol_Text]                      = Luma(100, 100);
        style.Colors[ImGuiCol_TextDisabled]              = Luma(39, 100);
        style.Colors[ImGuiCol_WindowBg]                  = grey1;
        style.Colors[ImGuiCol_ChildBg]                   = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
        style.Colors[ImGuiCol_PopupBg]                   = grey1;
        style.Colors[ImGuiCol_Border]                    = grey4;
        style.Colors[ImGuiCol_BorderShadow]              = grey1;
        style.Colors[ImGuiCol_FrameBg]                   = ImVec4(0.11f, 0.13f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered]            = grey4;
        style.Colors[ImGuiCol_FrameBgActive]             = grey4;
        style.Colors[ImGuiCol_TitleBg]                   = grey0;
        style.Colors[ImGuiCol_TitleBgActive]             = grey0;
        style.Colors[ImGuiCol_TitleBgCollapsed]          = grey1;
        style.Colors[ImGuiCol_MenuBarBg]                 = grey2;
        style.Colors[ImGuiCol_ScrollbarBg]               = grey0;
        style.Colors[ImGuiCol_ScrollbarGrab]             = grey3;
        style.Colors[ImGuiCol_ScrollbarGrabHovered]      = lo;
        style.Colors[ImGuiCol_ScrollbarGrabActive]       = hi;
        style.Colors[ImGuiCol_CheckMark]                 = alt;
        style.Colors[ImGuiCol_SliderGrab]                = lo;
        style.Colors[ImGuiCol_SliderGrabActive]          = hi;
        style.Colors[ImGuiCol_Button]                    = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered]             = lo;
        style.Colors[ImGuiCol_ButtonActive]              = grey5;
        style.Colors[ImGuiCol_Header]                    = grey3;
        style.Colors[ImGuiCol_HeaderHovered]             = lo;
        style.Colors[ImGuiCol_HeaderActive]              = hi;
        style.Colors[ImGuiCol_Separator]                 = ImVec4(0.13f, 0.15f, 0.19f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered]          = lo;
        style.Colors[ImGuiCol_SeparatorActive]           = hi;
        style.Colors[ImGuiCol_ResizeGrip]                = Luma(15, 100);
        style.Colors[ImGuiCol_ResizeGripHovered]         = lo;
        style.Colors[ImGuiCol_ResizeGripActive]          = hi;
        style.Colors[ImGuiCol_InputTextCursor]           = Luma(100, 100);
        style.Colors[ImGuiCol_TabHovered]                = grey3;
        style.Colors[ImGuiCol_Tab]                       = grey1;
        style.Colors[ImGuiCol_TabSelected]               = grey3;
        style.Colors[ImGuiCol_TabSelectedOverline]       = hi;
        style.Colors[ImGuiCol_TabDimmed]                 = grey1;
        style.Colors[ImGuiCol_TabDimmedSelected]         = grey1;
        style.Colors[ImGuiCol_TabDimmedSelectedOverline] = lo;
        style.Colors[ImGuiCol_DockingPreview]            = grey1;
        style.Colors[ImGuiCol_DockingEmptyBg]            = Luma(20, 100);
        style.Colors[ImGuiCol_PlotLines]                 = grey5;
        style.Colors[ImGuiCol_PlotLinesHovered]          = lo;
        style.Colors[ImGuiCol_PlotHistogram]             = grey5;
        style.Colors[ImGuiCol_PlotHistogramHovered]      = lo;
        style.Colors[ImGuiCol_TableHeaderBg]             = grey0;
        style.Colors[ImGuiCol_TableBorderStrong]         = grey0;
        style.Colors[ImGuiCol_TableBorderLight]          = grey0;
        style.Colors[ImGuiCol_TableRowBg]                = grey3;
        style.Colors[ImGuiCol_TableRowBgAlt]             = grey2;
        style.Colors[ImGuiCol_TextLink]                  = link;
        style.Colors[ImGuiCol_TextSelectedBg]            = Luma(39, 100);
        style.Colors[ImGuiCol_TreeLines]                 = Luma(39, 100);
        style.Colors[ImGuiCol_DragDropTarget]            = nav;
        style.Colors[ImGuiCol_NavCursor]                 = nav;
        style.Colors[ImGuiCol_NavWindowingHighlight]     = lo;
        style.Colors[ImGuiCol_NavWindowingDimBg]         = Luma(0, 63);
        style.Colors[ImGuiCol_ModalWindowDimBg]          = Luma(0, 63);

#undef Luma
    }

    void style_imgui_colors()
    {
        igThemeV3(7, 5, 7);
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
                      state = AppState::from_string(fileContent);
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

auto AppState::from_string(const string_view str) -> AppState
{
    const auto parsed = json::parse(str);

    AppState state;

    state.boardEditor = BoardEditorState::from_string(
        parsed.at(TAG_BOARD_EDITOR).get<string_view>());

    return state;
}

} // namespace ben_bot::gui
