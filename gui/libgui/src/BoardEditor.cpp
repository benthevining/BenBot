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
#include <beman/inplace_vector/inplace_vector.hpp>
#include <format>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Masks.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Patterns.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libgui/BoardEditor.hpp>
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

// TODOLIST :
// use ImGui::TextUnformatted() when passing output of std::format
// ApplicationState struct, top-level render_app() function. also initialize_app() function, move as much out of main.cpp as possible
// get rid of static vars in functions, pass structs around
// bug with removing duplicate EP squares
// render piece sprites in squares
// allow dropping dragged text into FEN input? BeginDragDropTarget(), AcceptDragDropPayload()
// sprite "trays" to click & drag pieces on to board
// ability to click & drag pieces on board to different squares. don't allow dropping piece on occupied square.
// ability to click & drag pieces on board off of board (or to a trash can icon?)
// windows build

namespace {
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

                    ImGui::Text("%s", std::format("{}", square).c_str());
                }
            }

            ImGui::EndTable();
        }
    }

    // This is different than legal move generation, which looks at the opponent's last move
    // to determine what EP captures are possible. Instead, this function looks only at the
    // positions of the pawns on the board to tell you which pawns could *potentially* legally
    // capture en passant, if the opponent had just moved the appropriate pawn.
    [[nodiscard, gnu::const]] auto get_possible_ep_squares(const Position& position) noexcept
    {
        using beman::inplace_vector::inplace_vector;

        const auto epRankMask = position.is_white_to_move()
                                  ? chess::board::masks::ranks::FIVE
                                  : chess::board::masks::ranks::FOUR;

        const auto possibleTakers = position.our_pieces().pawns & epRankMask;
        const auto posibleTaken   = position.their_pieces().pawns & epRankMask;

        auto squares = possibleTakers.subboards()
                     | std::views::transform([posibleTaken, isWhite = position.is_white_to_move()](
                                                 const chess::board::Bitboard startSquare) {
                           using chess::moves::patterns::pawn_attacks;
                           using chess::moves::patterns::pawn_pushes;

                           const auto attacks = isWhite
                                                  ? pawn_attacks<Color::White>(startSquare)
                                                  : pawn_attacks<Color::Black>(startSquare);

                           const auto takenMask = isWhite
                                                    ? pawn_pushes<Color::White>(posibleTaken)
                                                    : pawn_pushes<Color::Black>(posibleTaken);

                           return (attacks & takenMask).squares()
                                | std::ranges::to<inplace_vector<Square, 2uz>>();
                       })
                     | std::views::join
                     | std::ranges::to<inplace_vector<Square, 16uz>>();

        // TODO: bug here
        // filter duplicates
        std::ranges::sort(squares);

        squares.erase(
            std::ranges::unique(squares).end(),
            squares.end());

        return squares;
    }

    // When the user manually changes the side to move, we may need to reset the EP square
    // if it was set, because the en passant ranks are different for each side. This function
    // resets the EP square if it was set to one that is now illegal.
    void check_ep_square(Position& position)
    {
        position.enPassantTargetSquare.transform([&position](const Square prevEP) {
            if (not std::ranges::contains(get_possible_ep_squares(position), prevEP))
                position.enPassantTargetSquare.reset();

            return std::monostate { };
        });
    }

    void render_side_to_move(Position& position)
    {
        bool whiteToMove { position.sideToMove == Color::White };

        if (ImGui::Checkbox("White to move", &whiteToMove)) {
            position.sideToMove = whiteToMove ? Color::White : Color::Black;

            check_ep_square(position);
        }

        ImGui::SetItemTooltip("Set the side to move");
    }

    void render_castling_rights(Position& position)
    {
        ImGui::BeginGroup();

        ImGui::Text("Castling Rights");

        if (ImGui::BeginTable("CastlingRights", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
            static constexpr auto White = "White";
            static constexpr auto Black = "Black";

            static constexpr auto Kingside  = "O-O";
            static constexpr auto Queenside = "O-O-O";

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

    void render_ep_square(Position& position)
    {
        static constexpr auto NoneLabel = "None";

        const auto currEP = position.enPassantTargetSquare
                                .transform([](const Square square) { return std::format("{}", square); })
                                .value_or(std::string { NoneLabel });

        static std::optional<Square> selectedSquare;

        if (ImGui::BeginCombo("EPSquare", currEP.c_str(), ImGuiComboFlags_PopupAlignLeft)) {
            for (const auto square : get_possible_ep_squares(position)) {
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
        }

        ImGui::SetItemTooltip("Set the en passant target square");
    }

    void render_fen_string(Position& position)
    {
        static constexpr auto ErrorPopupID { "FEN parse error" };

        static std::string errorMessage;

        auto inputBuf = chess::notation::to_fen(position);

        if (ImGui::InputText("FEN", &inputBuf,
                ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
            [[maybe_unused]] const auto result
                = chess::notation::from_fen(inputBuf)
                      .transform([&position](const Position& newPos) {
                          position = newPos;
                          errorMessage.clear();
                          return std::monostate { };
                      })
                      .transform_error([](const std::string_view error) {
                          errorMessage = error;
                          ImGui::OpenPopup(ErrorPopupID, ImGuiPopupFlags_NoReopen);
                          return std::monostate { };
                      });
        }

        ImGui::SetItemTooltip("Enter a FEN string");

        if (ImGui::BeginPopupModal(ErrorPopupID, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", errorMessage.c_str());

            if (ImGui::Button("OK", { 120.f, 0.f })) {
                ImGui::CloseCurrentPopup();
                errorMessage.clear();
            }

            ImGui::EndPopup();
        }
    }
} // namespace

void board_editor(Position& position)
{
    if (ImGui::Begin("Board editor")) {
        render_chessboard(position);

        render_side_to_move(position);

        render_castling_rights(position);

        render_ep_square(position);

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

        render_fen_string(position);
    }

    ImGui::End();
}

} // namespace ben_bot::gui
