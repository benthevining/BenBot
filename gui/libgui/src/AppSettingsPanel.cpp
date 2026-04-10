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

#include "FileDialogs.hpp" // NOLINT(build/include_subdir)
#include "ImUtil.hpp"      // NOLINT(build/include_subdir)
#include <array>
#include <filesystem>
#include <format>
#include <imgui.h>
#include <libbenbot/Resources.hpp>
#include <libgui/AppSettingsPanel.hpp>
#include <libgui/AppUI.hpp>
#include <libgui/Resources.hpp>
#include <libutil/Files.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::gui {

using std::filesystem::path;
using std::string;
using std::string_view;

namespace {
    inline constexpr auto DefaultStateFilename { "AppState.json" };

    inline constexpr std::array StateFileFilters {
        file_dialog::Filter { "JSON", "json" }
    };

    void show_state_load_dialog(
        AppState& state)
    {
        file_dialog::show<true>(
            DefaultStateFilename, StateFileFilters,
            state.appSettings.defaultStateDirectory,
            [&state](const path& file) { state.load_from(file); });
    }

    void show_state_save_dialog(
        AppState& state)
    {
        file_dialog::show<false>(
            DefaultStateFilename, StateFileFilters,
            state.appSettings.defaultStateDirectory,
            [&state](const path& file) { state.write_to(file); });
    }

    void show_state_buttons(
        AppState& state)
    {
        const ScopedGroup group;

        const bool showTooltips = state.appSettings.showTooltips;

        if (ImGui::Button("Load state"))
            show_state_load_dialog(state);

        if (showTooltips)
            ImGui::SetItemTooltip("Load app state from a file");

        ImGui::SameLine();

        if (ImGui::Button("Save state"))
            show_state_save_dialog(state);

        if (showTooltips)
            ImGui::SetItemTooltip("Save app state to a file");

        if (ImGui::Button("Reset state"))
            state.update_from_string(resources::get_default_app_state());

        if (showTooltips)
            ImGui::SetItemTooltip("Reset app state to default");

        ImGui::SameLine();

        if (ImGui::Button("Reset layout"))
            load_default_ui_layout();

        if (showTooltips)
            ImGui::SetItemTooltip("Reset UI layout to default");
    }

    void show_build_info()
    {
        namespace res = ben_bot::resources;

        UnformattedText(
            std::format("BenBot version: {}", res::get_version_string()));

        UnformattedText(
            std::format("Compiler: {}", res::get_compiler_name()));

        UnformattedText(
            std::format("Compiler version: {}", res::get_compiler_version()));

        UnformattedText(
            std::format("Build configuration: {}", res::get_build_config()));
    }

    [[maybe_unused]] void show_default_state_save_buttons(
        AppState& state, const path& srcTreeResourcesPath)
    {
        ImGui::SeparatorText("Save default state");

        const ScopedGroup group;

        const bool showTooltips = state.appSettings.showTooltips;

        if (ImGui::Button("UI layout")) {
            ImGui::SaveIniSettingsToDisk(
                (srcTreeResourcesPath / "default_layout.ini").c_str());

            remove(
                get_default_imgui_ini_path());
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Save current UI layout as default in source tree");

        ImGui::SameLine();

        auto& saveError = state.appSettings.defaultStateSaveError;

        if (ImGui::Button("App state")) {
            [[maybe_unused]] const auto result
                = util::files::overwrite(
                    srcTreeResourcesPath / "default_state.json",
                    state.to_string())
                      .transform([&saveError] {
                          remove(
                              get_default_app_state_path());
                          saveError.set_success();
                          return std::monostate { };
                      })
                      .transform_error([&saveError](string&& message) {
                          saveError.set_error(std::move(message));
                          return std::monostate { };
                      });
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Save current app state as default in source tree");

        saveError.render();
    }
} // namespace

void render_app_settings_panel(AppState& state)
{
    if (ImGui::Begin("App settings")) {
        auto& showTooltips = state.appSettings.showTooltips;

        ImGui::Checkbox("Show tooltips", &showTooltips);

        if (showTooltips)
            ImGui::SetItemTooltip("Display widget tooltips");

        show_state_buttons(state);

        ImGui::Separator();

        show_build_info();

#ifdef BENBOT_RES_SRC_TREE_PATH
        show_default_state_save_buttons(
            state,
            path { BENBOT_RES_SRC_TREE_PATH });
#endif
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_TOOLTIPS { "show_tooltips" };

auto AppSettings::to_string() const -> string
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
