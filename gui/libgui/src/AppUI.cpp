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

    ImGui::StyleColorsDark();

    { // Setup scaling
        auto& style = ImGui::GetStyle();
        style.ScaleAllSizes(mainScaleFactor);
        style.FontScaleDpi = mainScaleFactor;

        style.WindowMenuButtonPosition = ImGuiDir_None;
        style.GrabRounding             = 4.0f;
        style.WindowRounding           = 4.0f;
        style.FrameRounding            = 4.0f;
        style.WindowBorderSize         = 0.0f;
        style.PopupBorderSize          = 0.0f;
        style.ChildBorderSize          = 0.0f;
        style.WindowMinSize            = { 200.0f, 200.0f };
    }

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
