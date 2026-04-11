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
#include <filesystem>
#include <format>
#include <imgui.h>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libgui/FileDialogContext.hpp>
#include <libgui/PSTEditor.hpp>
#include <libutil/Console.hpp>
#include <libutil/Files.hpp>
#include <magic_enum/magic_enum.hpp>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using Tables = eval::PieceSquareTables;

namespace {
    using magic_enum::enum_name;
    using magic_enum::enum_values;
    using std::filesystem::path;
    using std::string;

    void render_reset_button(
        Tables& tables, const bool showTooltips)
    {
        if (ImGui::Button("Reset"))
            tables = Tables::get_default();

        if (showTooltips)
            ImGui::SetItemTooltip("Reset to default BenBot tables");
    }

    void render_save_load_buttons(
        Tables& tables, FileDialogContext& context, const bool showTooltips)
    {
        const ScopedGroup group;

        if (ImGui::Button("Load")) {
            context.load_file([&tables](const path& file) {
                [[maybe_unused]] const auto result
                    = util::files::load(file)
                          .transform([&tables](const std::string_view text) {
                              tables = Tables::from_string(text);
                              return std::monostate { };
                          })
                          .transform_error(util::print_error);
            });
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Load table data from file");

        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            context.save_file([&tables](const path& file) {
                [[maybe_unused]] const auto result
                    = util::files::overwrite(file, tables.to_string())
                          .transform_error(util::print_error);
            });
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Save table data to file");
    }

    void render_engine_interop_buttons(
        Tables& tables, Engine& engine, const bool showTooltips)
    {
        const ScopedGroup group;

        if (ImGui::Button("Send to engine"))
            engine.set_piece_square_tables(tables);

        if (showTooltips)
            ImGui::SetItemTooltip("Send tables to engine (interrupts search if active)");

        ImGui::SameLine();

        if (ImGui::Button("Refresh from engine"))
            tables = engine.get_piece_square_tables();

        if (showTooltips)
            ImGui::SetItemTooltip("Reset to engine's current tables");
    }

    void render_table_type_selector(
        Tables::TableType& selectedType, const bool showTooltips)
    {
        const string currentType { enum_name(selectedType) };

        if (ImGui::BeginCombo("Type", currentType.c_str())) {
            for (const auto type : enum_values<Tables::TableType>()) {
                const bool isSelected = type == selectedType;

                if (ImGui::Selectable(string { enum_name(type) }.c_str(), isSelected))
                    selectedType = type;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Select table type to edit");
    }

    void render_value_grid(
        Tables& tables, const Tables::TableType selectedType)
    {
        if (not ImGui::BeginTable("PSTValues", 8, ImGuiTableFlags_Borders))
            return;

        for (const auto file : enum_values<chess::board::File>()) {
            ImGui::TableSetupColumn(
                string { enum_name(file) }.c_str());
        }

        ImGui::TableHeadersRow();

        auto& table = tables.get_table(selectedType);

        for (const auto rank : enum_values<chess::board::Rank>()) {
            ImGui::TableNextRow();

            for (const auto file : enum_values<chess::board::File>()) {
                ImGui::TableNextColumn();

                const chess::board::Square square {
                    .file = file, .rank = rank
                };

                ImGui::InputInt(
                    std::format("{}", square).c_str(),
                    &table.at(square.index()),
                    0);
            }
        }

        ImGui::EndTable();
    }

    void render_pst_values(
        Tables& tables, Tables::TableType& selectedType, const bool showTooltips)
    {
        render_table_type_selector(
            selectedType, showTooltips);

        render_value_grid(
            tables, selectedType);
    }
} // namespace

void render_pst_editor(
    PSTEditorState& state, Engine& engine, const bool showTooltips)
{
    if (ImGui::Begin("Piece square tables")) {
        // overwrite default in source tree

        render_reset_button(
            state.tables, showTooltips);

        render_save_load_buttons(
            state.tables, state.loadSave, showTooltips);

        render_engine_interop_buttons(
            state.tables, engine, showTooltips);

        render_pst_values(
            state.tables, state.selectedType, showTooltips);
    }

    ImGui::End();
}

} // namespace ben_bot::gui
