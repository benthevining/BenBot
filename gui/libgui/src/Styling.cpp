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
#include <imgui.h>

namespace ben_bot::gui {

namespace {
    void setup_benbot_colors()
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

        auto& colors = ImGui::GetStyle().Colors;

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
        colors[ImGuiCol_DockingPreview]        = lavender;
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
    }
} // namespace

void setup_imgui_style()
{
    setup_benbot_colors();

    auto& style = ImGui::GetStyle();

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

    // TODO: load custom fonts
}

} // namespace ben_bot::gui
