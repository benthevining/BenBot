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

#include <algorithm>
#include <array>
#include <beman/inplace_vector/inplace_vector.hpp>
#include <cassert>
#include <format>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <iterator>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libgui/BoardEditor.hpp>
#include <limits>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::pieces::Color;

using chess::moves::get_potentially_legal_en_passant_target_squares;

namespace {
    void UnformattedText(const std::string_view text)
    {
        ImGui::TextUnformatted(
            text.data(),
            std::next(
                text.data(),
                static_cast<std::ptrdiff_t>(text.length())));
    }

    void render_chessboard(Position& position)
    {
        static constexpr auto colorWhite = IM_COL32(255, 255, 255, 255);
        static constexpr auto colorBlack = IM_COL32(0, 0, 0, 255);

        static constexpr auto squareSize = 50.f;

        if (ImGui::BeginTable("Board", 8, ImGuiTableFlags_SizingFixedSame)) {
            for (const auto file : magic_enum::enum_values<File>()) {
                ImGui::TableSetupColumn(
                    std::format("{}", file).c_str(),
                    ImGuiTableColumnFlags_WidthFixed, squareSize);
            }

            for (const auto rank : magic_enum::enum_values<Rank>() | std::views::reverse) {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, squareSize);

                for (const auto file : magic_enum::enum_values<File>()) {
                    ImGui::TableNextColumn();

                    const Square square { .file = file, .rank = rank };

                    ImGui::TableSetBgColor(
                        ImGuiTableBgTarget_CellBg,
                        square.is_light() ? colorWhite : colorBlack);

                    UnformattedText(std::format("{}", square));
                }
            }

            ImGui::EndTable();
        }
    }

    void render_side_to_move(Position& position)
    {
        bool whiteToMove { position.sideToMove == Color::White };

        if (ImGui::Checkbox("White to move", &whiteToMove)) {
            position.sideToMove = whiteToMove ? Color::White : Color::Black;

            // When the user manually changes the side to move, we may need to reset the EP square
            // if it was set, because the en passant ranks are different for each side. This function
            // resets the EP square if it was set to one that is now illegal.
            position.sanitize_ep_square();
        }

        ImGui::SetItemTooltip("Set the side to move");
    }

    void render_castling_rights(Position& position)
    {
        static constexpr auto TableFlags       = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit;
        static constexpr auto TableColumnFlags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;

        static constexpr auto White = "White";
        static constexpr auto Black = "Black";

        static constexpr auto Kingside  = "O-O";
        static constexpr auto Queenside = "O-O-O";

        ImGui::BeginGroup();

        ImGui::Text("Castling Rights");

        if (ImGui::BeginTable("CastlingRights", 3, TableFlags,
                { ImGui::CalcTextSize(Queenside).x * 5.f, 0.f })) {
            ImGui::TableSetupColumn("ColorLabels", TableColumnFlags,
                ImGui::CalcTextSize(White).x + 5.f);

            ImGui::TableSetupColumn(Kingside, TableColumnFlags);
            ImGui::TableSetupColumn(Queenside, TableColumnFlags);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text(White);

            ImGui::TableNextColumn();
            ImGui::Checkbox(
                std::format("{}##{}", Kingside, White).c_str(),
                &position.whiteCastlingRights.kingside);

            ImGui::TableNextColumn();
            ImGui::Checkbox(
                std::format("{}##{}", Queenside, White).c_str(),
                &position.whiteCastlingRights.queenside);

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text(Black);

            ImGui::TableNextColumn();
            ImGui::Checkbox(
                std::format("{}##{}", Kingside, Black).c_str(),
                &position.blackCastlingRights.kingside);

            ImGui::TableNextColumn();
            ImGui::Checkbox(
                std::format("{}##{}", Queenside, Black).c_str(),
                &position.blackCastlingRights.queenside);

            ImGui::EndTable();
        }

        ImGui::EndGroup();

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Set the castling rights of each side");
    }

    void render_ep_square(
        Position& position, std::optional<Square>& selectedSquare)
    {
        static constexpr auto ComboFlags = ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_WidthFitPreview;

        static constexpr auto NoneLabel = "None";

        const auto currEP = position.enPassantTargetSquare
                                .transform([](const Square square) { return std::format("{}", square); })
                                .value_or(std::string { NoneLabel });

        if (ImGui::BeginCombo("EPSquare", currEP.c_str(), ComboFlags)) {
            for (const auto square : get_potentially_legal_en_passant_target_squares(position)) {
                const bool isSelected = selectedSquare.has_value() and square == *selectedSquare;

                if (ImGui::Selectable(std::format("{}", square).c_str(), isSelected)) {
                    selectedSquare                 = square;
                    position.enPassantTargetSquare = selectedSquare;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            // "none" option
            const bool isSelected = not selectedSquare.has_value();

            if (ImGui::Selectable(NoneLabel, isSelected)) {
                selectedSquare.reset();
                position.enPassantTargetSquare.reset();
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();

            ImGui::EndCombo();
        } else {
            selectedSquare = position.enPassantTargetSquare;
        }

        ImGui::SetItemTooltip("Set the en passant target square");
    }

    // reset, flip, clear buttons
    void render_utility_buttons(Position& position)
    {
        ImGui::BeginGroup();

        if (ImGui::Button("Reset"))
            position = Position { };

        ImGui::SetItemTooltip("Reset the board to the starting position");

        ImGui::SameLine();

        if (ImGui::Button("Flip"))
            position = flipped(position);

        ImGui::SetItemTooltip("Flip the board vertically");

        ImGui::SameLine();

        if (ImGui::Button("Clear"))
            position = Position::empty();

        ImGui::SetItemTooltip("Remove all pieces from the board");

        ImGui::EndGroup();
    }

    void render_fen_string(
        Position& position, std::string& errorMessage)
    {
        static constexpr auto InputTextFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
        static constexpr auto PopupFlags     = ImGuiWindowFlags_AlwaysAutoResize;

        static constexpr auto ErrorPopupID { "FEN parse error" };

        auto inputBuf = chess::notation::to_fen(position);

        ImGui::SetNextItemWidth(ImGui::CalcTextSize(inputBuf.c_str()).x + 10.f);

        if (ImGui::InputText("FEN", &inputBuf, InputTextFlags)) {
            [[maybe_unused]] const auto result
                = chess::notation::from_fen(inputBuf)
                      .transform([&position, &errorMessage](const Position& newPos) {
                          position = newPos;
                          errorMessage.clear();
                          return std::monostate { };
                      })
                      .transform_error([&errorMessage](const std::string_view error) {
                          assert(not error.empty());
                          errorMessage = error;
                          ImGui::OpenPopup(ErrorPopupID, ImGuiPopupFlags_NoReopen);
                          return std::monostate { };
                      });
        }

        ImGui::SetItemTooltip("Enter a FEN string");

        if (ImGui::BeginPopupModal(ErrorPopupID, nullptr, PopupFlags)) {
            UnformattedText(errorMessage);

            if (ImGui::Button("OK", { 120.f, 0.f })) {
                ImGui::CloseCurrentPopup();
                errorMessage.clear();
            }

            ImGui::EndPopup();
        }
    }
} // namespace

void board_editor(BoardEditorState& state)
{
    static constexpr auto MAX_SIZE = std::numeric_limits<float>::max();

    ImGui::SetNextWindowSizeConstraints(
        { 475.f, 595.f },
        { MAX_SIZE, MAX_SIZE });

    if (ImGui::Begin("Board editor")) {
        // render_chessboard(state.position);

        render_side_to_move(state.position);

        render_castling_rights(state.position);

        render_ep_square(state.position, state.selectedEPSquare);

        render_utility_buttons(state.position);

        render_fen_string(state.position, state.fenParseError);
    }

    ImGui::End();
}

} // namespace ben_bot::gui
