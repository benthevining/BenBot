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
#include <array>
#include <concepts>
#include <cstdio>
#include <filesystem>
#include <format>
#include <imgui.h>
#include <libbenbot/Resources.hpp>
#include <libgui/AppSettingsPanel.hpp>
#include <libgui/AppUI.hpp>
#include <libgui/Resources.hpp>
#include <nfd.hpp>
#include <nlohmann/json.hpp>
#include <print>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using std::filesystem::path;
using std::string_view;

namespace {
    template <typename Func>
    concept FileCallback = std::regular_invocable<Func, const path&>;

    template <bool IsLoading>
    void show_file_dialog(FileCallback auto callback)
    {
        NFD::UniquePath outPath;

        const auto result = [&outPath]() noexcept {
            static constexpr std::array filters {
                nfdu8filteritem_t { "JSON", "json" }
            };

            if constexpr (IsLoading) {
                return OpenDialog(
                    outPath,
                    filters.data(), filters.size());
            } else {
                return SaveDialog(
                    outPath,
                    filters.data(), filters.size(),
                    nullptr, "AppState.json");
            }
        }();

        switch (result) {
            case NFD_OKAY:
                callback(path { outPath.get() });
                break;

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

    void show_state_load_dialog(AppState& state)
    {
        show_file_dialog<true>(
            [&state](const path& file) { state.load_from(file); });
    }

    void show_state_save_dialog(const AppState& state)
    {
        show_file_dialog<false>(
            [&state](const path& file) { state.write_to(file); });
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

        ImGui::SameLine();

        if (ImGui::Button("Save state"))
            show_state_save_dialog(state);

        if (showTooltips)
            ImGui::SetItemTooltip("Save app state to a file");

        ImGui::SameLine();

        if (ImGui::Button("Reset state"))
            state.update_from_string(resources::get_default_app_state());

        if (showTooltips)
            ImGui::SetItemTooltip("Reset app state to default");

        ImGui::Separator();

        show_build_info();
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
