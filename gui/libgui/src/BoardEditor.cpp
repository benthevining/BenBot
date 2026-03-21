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
#include <cstdint>
#include <format>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <iterator>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libgui/BoardEditor.hpp>
#include <libutil/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::game::Position;
using chess::pieces::Color;
using std::string;
using std::string_view;

using chess::moves::get_potentially_legal_en_passant_target_squares;

namespace {
    inline constexpr auto CollapsibleFlags = ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_Framed;
    inline constexpr auto InputTextFlags   = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
    inline constexpr auto PopupFlags       = ImGuiWindowFlags_AlwaysAutoResize;

    void UnformattedText(const string_view text)
    {
        ImGui::TextUnformatted(
            text.data(),
            std::next(
                text.data(),
                static_cast<std::ptrdiff_t>(text.length())));
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
        static constexpr auto White = "White";
        static constexpr auto Black = "Black";

        static constexpr auto Kingside  = "O-O";
        static constexpr auto Queenside = "O-O-O";

        if (ImGui::CollapsingHeader("Castling Rights", CollapsibleFlags)) {
            ImGui::BeginGroup();

            if (ImGui::BeginTable("CastlingRights", 3, ImGuiTableFlags_Borders)) {
                ImGui::TableSetupColumn("ColorLabels");
                ImGui::TableSetupColumn(Kingside);
                ImGui::TableSetupColumn(Queenside);

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
    }

    void render_ep_square(
        Position& position, std::optional<Square>& selectedSquare)
    {
        static constexpr auto NoneLabel = "None";

        const auto currEP = position.enPassantTargetSquare
                                .transform([](const Square square) { return std::format("{}", square); })
                                .value_or(string { NoneLabel });

        if (ImGui::BeginCombo("EPSquare", currEP.c_str())) {
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
    void render_utility_buttons(EPDPosition& position)
    {
        ImGui::BeginGroup();

        if (ImGui::Button("Reset"))
            position = EPDPosition { };

        ImGui::SetItemTooltip("Reset the board to the starting position");

        ImGui::SameLine();

        if (ImGui::Button("Flip"))
            position.position = flipped(position.position);

        ImGui::SetItemTooltip("Flip the board vertically");

        ImGui::SameLine();

        if (ImGui::Button("Clear"))
            position.position = Position::empty();

        ImGui::SetItemTooltip("Remove all pieces from the board");

        ImGui::EndGroup();
    }

    void render_move_counters(EPDPosition& position)
    {
        static constexpr auto HalfMovesLabel = "Half moves";
        static constexpr auto FullMovesLabel = "Full moves";

        ImGui::BeginGroup();

        const auto IntEntryWidth = ImGui::CalcTextSize(HalfMovesLabel).x * 1.25f;

        ImGui::SetNextItemWidth(IntEntryWidth);

        auto halfMoveCounter = static_cast<int>(position.position.halfmoveClock);

        if (ImGui::InputInt(HalfMovesLabel, &halfMoveCounter)) {
            position.position.halfmoveClock = static_cast<std::uint_least8_t>(
                std::clamp(halfMoveCounter, 0, 100));
            position.refresh_default_operations();
        }

        ImGui::SetItemTooltip(
            "This counter enforces the 50-move rule; this is the number of plies since the last capture or pawn move.");

        ImGui::SameLine();

        ImGui::SetNextItemWidth(IntEntryWidth);

        auto fullMoveCounter = static_cast<int>(position.position.fullMoveCounter);

        if (ImGui::InputInt(FullMovesLabel, &fullMoveCounter)) {
            position.position.fullMoveCounter = static_cast<std::uint_least64_t>(
                std::max(fullMoveCounter, 1));
            position.refresh_default_operations();
        }

        ImGui::SetItemTooltip(
            "The number of full turns in the game so far. This counter is incremented after each Black move.");

        ImGui::EndGroup();
    }

    void render_fen_string(
        Position& position, string& errorMessage)
    {
        using chess::notation::from_fen;
        using chess::notation::to_fen;

        static constexpr auto ErrorPopupID { "FEN parse error" };

        auto inputBuf = to_fen(position);

        if (ImGui::InputText("FEN", &inputBuf, InputTextFlags)) {
            [[maybe_unused]] const auto result
                = from_fen(inputBuf)
                      .transform([&position, &errorMessage](const Position& newPos) {
                          position = newPos;
                          errorMessage.clear();
                          return std::monostate { };
                      })
                      .transform_error([&errorMessage](const string_view error) {
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

    using chess::notation::from_epd;
    using chess::notation::to_epd;

    void render_epd_string(EPDPosition& position, string& errorMessage)
    {
        static constexpr auto ErrorPopupID { "EPD parse error" };

        auto inputBuf = to_epd(position);

        if (ImGui::InputText("EPD", &inputBuf, InputTextFlags)) {
            [[maybe_unused]] const auto result
                = from_epd(inputBuf)
                      .transform([&position, &errorMessage](const EPDPosition& newPos) {
                          position = newPos;
                          errorMessage.clear();
                          return std::monostate { };
                      })
                      .transform_error([&errorMessage](const string_view error) {
                          assert(not error.empty());
                          errorMessage = error;
                          ImGui::OpenPopup(ErrorPopupID, ImGuiPopupFlags_NoReopen);
                          return std::monostate { };
                      });
        }

        ImGui::SetItemTooltip("Enter an EPD string");

        if (ImGui::BeginPopupModal(ErrorPopupID, nullptr, PopupFlags)) {
            UnformattedText(errorMessage);

            if (ImGui::Button("OK", { 120.f, 0.f })) {
                ImGui::CloseCurrentPopup();
                errorMessage.clear();
            }

            ImGui::EndPopup();
        }
    }

    void render_epd_operations(EPDPosition& position)
    {
        // see https://www.chessprogramming.org/Extended_Position_Description

        // TODO: maybe a move type?
        // TODO: Maybe an IntegerNonNegative type?
        enum class Type {
            Integer,
            String
        };

        struct StandardOperation final {
            string mnemonic;
            Type   type;
            string tooltip;
        };

        // TODO: c0 comment (primary, also c1 though c9), draw offer/accept, v0 variation name (primary, also v1 though v9)
        static const std::array StandardOperations {
            StandardOperation { .mnemonic = "acd", .type = Type::Integer, .tooltip = "Analysis count depth" },
            StandardOperation { .mnemonic = "acn", .type = Type::Integer, .tooltip = "Analysis count nodes" }, // codespell:ignore acn
            StandardOperation { .mnemonic = "acs", .type = Type::Integer, .tooltip = "Analysis count seconds" },
            StandardOperation { .mnemonic = "am", .type = Type::String, .tooltip = "Avoid move(s)" },
            StandardOperation { .mnemonic = "bm", .type = Type::String, .tooltip = "Best move(s)" },
            StandardOperation { .mnemonic = "ce", .type = Type::Integer, .tooltip = "Centipawn evaluation" },
            StandardOperation { .mnemonic = "dm", .type = Type::Integer, .tooltip = "Direct mate fullmove count" },
            StandardOperation { .mnemonic = "eco", .type = Type::String, .tooltip = "Encyclopedia of Chess Openings opening code" },
            StandardOperation { .mnemonic = "id", .type = Type::String, .tooltip = "Position identification" },
            StandardOperation { .mnemonic = "nic", .type = Type::String, .tooltip = "New In Chess opening code" },
            StandardOperation { .mnemonic = "pm", .type = Type::String, .tooltip = "Predicted move" },
            StandardOperation { .mnemonic = "pv", .type = Type::String, .tooltip = "Predicted variation" },
            StandardOperation { .mnemonic = "rc", .type = Type::Integer, .tooltip = "Repetition count" },
            StandardOperation { .mnemonic = "sm", .type = Type::String, .tooltip = "Supplied move" }
        };

        for (const auto& opMetadata : StandardOperations) {
            auto inputBuf = [&position, &opMetadata] {
                const auto it = position.operations.find(opMetadata.mnemonic);

                if (it != position.operations.end())
                    return it->second;

                return string { };
            }();

            switch (opMetadata.type) {
                case Type::Integer: {
                    auto value = util::strings::int_from_string<int>(inputBuf);

                    if (ImGui::InputInt(opMetadata.mnemonic.c_str(), &value, 1, 10, ImGuiInputTextFlags_CharsDecimal)) {
                        position.operations[opMetadata.mnemonic] = std::format("{}", value);
                    }
                    break;
                }

                default: [[fallthrough]];
                case Type::String:
                    if (ImGui::InputText(opMetadata.mnemonic.c_str(), &inputBuf, InputTextFlags)) {
                        position.operations[opMetadata.mnemonic] = util::strings::trim(inputBuf);
                    }
            }

            ImGui::SetItemTooltip("%s", opMetadata.tooltip.c_str());
        }
    }

    void render_epd_editor(
        EPDPosition& position, string& errorMessage)
    {
        if (ImGui::CollapsingHeader("EPD editor", CollapsibleFlags)) {
            render_epd_string(position, errorMessage);
            render_epd_operations(position);
        }
    }
} // namespace

void render_board_editor(BoardEditorState& state)
{
    if (ImGui::Begin("Board editor")) {
        render_utility_buttons(state.position);

        render_side_to_move(state.position.position);

        render_castling_rights(state.position.position);

        render_ep_square(state.position.position, state.selectedEPSquare);

        render_move_counters(state.position);

        render_fen_string(state.position.position, state.fenParseError);

        render_epd_editor(state.position, state.epdParseError);
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_CURRENTBOARD { "board_fen" };

auto BoardEditorState::to_string() const -> string
{
    json data;

    data[TAG_CURRENTBOARD] = to_epd(position);

    return data.dump();
}

auto BoardEditorState::from_string(string_view str) -> BoardEditorState
{
    const auto parsed = json::parse(str);

    BoardEditorState state;

    [[maybe_unused]] const auto result
        = from_epd(parsed.at(TAG_CURRENTBOARD).get<string_view>())
              .transform([&state](const EPDPosition& newPos) {
                  state.position = newPos;
                  return std::monostate { };
              });

    return state;
}

} // namespace ben_bot::gui
