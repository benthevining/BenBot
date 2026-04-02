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

#include <array>
#include <cstdio>
#include <filesystem>
#include <imgui.h>
#include <libgui/AppSettingsPanel.hpp>
#include <libgui/AppUI.hpp>
#include <nfd.hpp>
#include <nlohmann/json.hpp>
#include <print>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using std::string_view;

static constexpr std::array StateFileChooserFilters {
    nfdu8filteritem_t { "JSON", "json" }
};

namespace {
    void show_state_load_dialog(AppState& state)
    {
        nfdopendialogu8args_t args { };
        args.filterList  = StateFileChooserFilters.data();
        args.filterCount = StateFileChooserFilters.size();

        nfdu8char_t* outPath { nullptr };

        switch (NFD_OpenDialogU8_With(&outPath, &args)) {
            case NFD_OKAY: {
                state.load_from(std::filesystem::path { outPath });
                NFD_FreePathU8(outPath);
                break;
            }

            case NFD_CANCEL:
                std::println("Info: user canceled file selection dialog");
                break;

            case NFD_ERROR:
                std::println(
                    stderr, "Info: error with file selection dialog");
                break;

            default: break;
        }
    }

    void show_state_save_dialog(const AppState& state)
    {
        nfdsavedialogu8args_t args { };
        args.filterList  = StateFileChooserFilters.data();
        args.filterCount = StateFileChooserFilters.size();

        nfdu8char_t* outPath { nullptr };

        switch (NFD_SaveDialogU8_With(&outPath, &args)) {
            case NFD_OKAY: {
                state.write_to(std::filesystem::path { outPath });
                NFD_FreePathU8(outPath);
                break;
            }

            case NFD_CANCEL:
                std::println("Info: user canceled file selection dialog");
                break;

            case NFD_ERROR:
                std::println(
                    stderr, "Info: error with file selection dialog");
                break;

            default: break;
        }
    }
} // namespace

void render_app_settings_panel(AppState& state)
{
    if (ImGui::Begin("App settings")) {
        auto& showTooltips = state.appSettings.showTooltips;

        ImGui::Checkbox("Show tooltips", &showTooltips);

        if (showTooltips)
            ImGui::SetItemTooltip("Display widget tooltips");

        if (ImGui::Button("Load state"))
            show_state_load_dialog(state);

        if (showTooltips)
            ImGui::SetItemTooltip("Load app state from a file");

        if (ImGui::Button("Save state"))
            show_state_save_dialog(state);

        if (showTooltips)
            ImGui::SetItemTooltip("Save app state to a file");
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_TOOLTIPS { "show_tooltips" };

auto AppSettings::to_string() const -> std::string
{
    json data;

    data[TAG_TOOLTIPS] = showTooltips;

    return data.dump();
}

auto AppSettings::from_string(const string_view str) -> AppSettings
{
    const auto parsed = json::parse(str);

    return AppSettings {
        .showTooltips = parsed.at(TAG_TOOLTIPS).get<bool>()
    };
}

} // namespace ben_bot::gui
