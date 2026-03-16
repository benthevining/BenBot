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

#include <format>
#include <imgui.h>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libgui/BoardEditor.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>

namespace ben_bot::gui {

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

void board_editor(Position& position)
{
    static constexpr auto colorWhite = IM_COL32(255, 255, 255, 255);
    static constexpr auto colorBlack = IM_COL32(0, 0, 0, 255);

    ImGui::Begin("Board editor");

    const auto [width, height] = ImGui::GetWindowSize();

    const auto squareSize = std::min(
        width / 8.f, height / 8.f);

    if (ImGui::BeginTable("Board", 8, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
        for (const auto file : magic_enum::enum_values<File>()) {
            ImGui::TableSetupColumn(
                std::format("{}", file).data(),
                ImGuiTableColumnFlags_WidthFixed, squareSize);
        }

        for (const auto rank : magic_enum::enum_values<Rank>() | std::views::reverse) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, squareSize);

            for (const auto file : magic_enum::enum_values<File>()) {
                ImGui::TableNextColumn();

                const Square square {
                    .file = file,
                    .rank = rank
                };

                ImGui::TableSetBgColor(
                    ImGuiTableBgTarget_CellBg,
                    square.is_light() ? colorWhite : colorBlack);

                ImGui::Text("%s", std::format("{}", square).data());
            }
        }

        ImGui::EndTable(); // Must be called to finish the table
    }

    ImGui::End();
}

} // namespace ben_bot::gui
