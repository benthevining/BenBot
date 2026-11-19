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
#include <imgui.h>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <libgui/ChessboardWidget.hpp>
#include <libgui/PieceTextures.hpp>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <utility>

namespace ben_bot::gui {

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

namespace {
    constexpr auto LightSquareColor = IM_COL32(240, 217, 181, 255);
    constexpr auto DarkSquareColor  = IM_COL32(181, 136, 99, 255);

    constexpr auto MinSquareSize = 24.f;

    // fraction of the square size to inset the coordinate labels by
    constexpr auto LabelPadding = 0.04f;

    [[nodiscard]] auto get_board_size() -> float
    {
        const auto avail = ImGui::GetContentRegionAvail();

        return std::max(
            std::min(avail.x, avail.y),
            MinSquareSize * 8.f);
    }

    // lowest rank is at the bottom
    [[nodiscard]] auto get_square_min(
        const ImVec2 boardOrigin, const Square square, const float squareSize) -> ImVec2
    {
        static constexpr auto LastRank = std::to_underlying(Rank::Eight);

        return {
            boardOrigin.x + (static_cast<float>(std::to_underlying(square.file)) * squareSize),
            boardOrigin.y + (static_cast<float>(LastRank - std::to_underlying(square.rank)) * squareSize)
        };
    }

    void draw_piece(
        ImDrawList&          drawList,
        const PieceTextures& pieceTextures,
        const PieceType type, const Color color,
        const ImVec2 min, const ImVec2 max)
    {
        if (pieceTextures.is_valid()) {
            const auto uvs = get_sprite_uvs(type, color);

            drawList.AddImage(
                ImTextureRef { pieceTextures.textureID },
                min, max,
                { uvs.u0, uvs.v0 }, { uvs.u1, uvs.v1 });

            return;
        }

        const bool isWhite = color == Color::White;

        const std::array text { chess::pieces::to_char(type, isWhite), '\0' };

        const auto textSize = ImGui::CalcTextSize(text.data());

        drawList.AddText(
            { min.x + ((max.x - min.x - textSize.x) * 0.5f),
                min.y + ((max.y - min.y - textSize.y) * 0.5f) },
            isWhite ? IM_COL32_WHITE : IM_COL32_BLACK,
            text.data());
    }

    // draws the file letters along the first rank and the rank numbers along the A-file,
    // in the color of the opposite square so that they contrast with the square they're on
    void draw_coordinate_label(
        ImDrawList& drawList, const Square square,
        const ImVec2 min, const ImVec2 max)
    {
        const auto padding = (max.x - min.x) * LabelPadding;

        const auto color = square.is_light() ? DarkSquareColor : LightSquareColor;

        if (square.file == File::A) {
            const std::array text { chess::board::rank_to_char(square.rank), '\0' };

            drawList.AddText(
                { min.x + padding, min.y + padding },
                color, text.data());
        }

        if (square.rank == Rank::One) {
            const std::array text { chess::board::file_to_char(square.file), '\0' };

            const auto textSize = ImGui::CalcTextSize(text.data());

            drawList.AddText(
                { max.x - textSize.x - padding, max.y - textSize.y - padding },
                color, text.data());
        }
    }
} // namespace

void render_chessboard(
    const Position&      position,
    const PieceTextures& pieceTextures)
{
    const auto boardSize  = get_board_size();
    const auto squareSize = boardSize / 8.f;

    const auto boardOrigin = ImGui::GetCursorScreenPos();

    auto& drawList = *ImGui::GetWindowDrawList();

    ImGui::PushID("Chessboard");

    // iterate top to bottom, so that the cursor ends up below the board
    for (const auto rank : magic_enum::enum_values<Rank>() | std::views::reverse) {
        for (const auto file : magic_enum::enum_values<File>()) {
            const Square square { .file = file, .rank = rank };

            const auto   min = get_square_min(boardOrigin, square, squareSize);
            const ImVec2 max { min.x + squareSize, min.y + squareSize };

            ImGui::SetCursorScreenPos(min);

            ImGui::PushID(static_cast<int>(square.index()));
            ImGui::InvisibleButton("square", { squareSize, squareSize });
            ImGui::PopID();

            drawList.AddRectFilled(
                min, max,
                square.is_light() ? LightSquareColor : DarkSquareColor);

            if (const auto type = position.whitePieces.get_piece_on(square))
                draw_piece(drawList, pieceTextures, *type, Color::White, min, max);
            else if (const auto blackType = position.blackPieces.get_piece_on(square))
                draw_piece(drawList, pieceTextures, *blackType, Color::Black, min, max);

            draw_coordinate_label(drawList, square, min, max);
        }
    }

    ImGui::PopID();
}

} // namespace ben_bot::gui
