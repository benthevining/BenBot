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
#include <algorithm>
#include <array>
#include <beman/inplace_vector/inplace_vector.hpp>
#include <cassert>
#include <cstdint> // IWYU pragma: keep
#include <format>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libgui/BoardEditor.hpp>
#include <libgui/EngineWrapper.hpp>
#include <libutil/Console.hpp>
#include <libutil/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using chess::board::Square;
using chess::game::Position;
using chess::pieces::Color;
using std::string;
using std::string_view;

using chess::moves::get_potentially_legal_en_passant_target_squares;

namespace {
    // reset, flip, clear buttons
    void render_utility_buttons(
        EPDPosition& position, const bool showTooltips)
    {
        const ScopedGroup group;

        if (ImGui::Button("Reset"))
            position = EPDPosition { };

        if (showTooltips)
            ImGui::SetItemTooltip("Reset the board to the starting position");

        ImGui::SameLine();

        if (ImGui::Button("Clear"))
            position.position = Position::empty();

        if (showTooltips)
            ImGui::SetItemTooltip("Remove all pieces from the board");

        ImGui::SameLine();

        if (ImGui::Button("Flip"))
            position.position.flip();

        if (showTooltips)
            ImGui::SetItemTooltip("Flip the board vertically");

        ImGui::SameLine();

        if (ImGui::Button("Make null"))
            position.position.make_null_move();

        if (showTooltips)
            ImGui::SetItemTooltip("Make a null move on the board");
    }

    void render_side_to_move(
        Position& position, const bool showTooltips)
    {
        bool whiteToMove { position.sideToMove == Color::White };

        if (ImGui::Checkbox("White to move", &whiteToMove)) {
            position.sideToMove = whiteToMove ? Color::White : Color::Black;

            // When the user manually changes the side to move, we may need to reset the EP square
            // if it was set, because the en passant ranks are different for each side. This function
            // resets the EP square if it was set to one that is now illegal.
            position.sanitize_ep_square();

            position.refresh_zobrist();
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Set the side to move");
    }

    void render_castling_rights(
        Position& position, const bool showTooltips)
    {
        static constexpr auto White = "White";
        static constexpr auto Black = "Black";

        static constexpr auto Kingside  = "O-O";
        static constexpr auto Queenside = "O-O-O";

        if (not ImGui::CollapsingHeader("Castling Rights"))
            return;

        const ScopedGroup group;

        if (ImGui::BeginTable("CastlingRights", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("ColorLabels");
            ImGui::TableSetupColumn(Kingside);
            ImGui::TableSetupColumn(Queenside);

            {
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
            }
            {
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
            }

            ImGui::EndTable();

            position.refresh_zobrist();
        }

        if (showTooltips and ImGui::IsItemHovered())
            ImGui::SetTooltip("Set the castling rights of each side");
    }

    void render_ep_square(
        Position& position, std::optional<Square>& selectedSquare, const bool showTooltips)
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
                    position.refresh_zobrist();
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            // "none" option
            const bool isSelected = not selectedSquare.has_value();

            if (ImGui::Selectable(NoneLabel, isSelected)) {
                selectedSquare.reset();
                position.enPassantTargetSquare.reset();
                position.refresh_zobrist();
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();

            ImGui::EndCombo();
        } else {
            selectedSquare = position.enPassantTargetSquare;
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Set the en passant target square");
    }

    void render_move_counters(
        EPDPosition& position, const bool showTooltips)
    {
        static constexpr auto HalfMovesLabel = "Half moves";
        static constexpr auto FullMovesLabel = "Full moves";

        const ScopedGroup group;

        const auto IntEntryWidth = ImGui::CalcTextSize(HalfMovesLabel).x * 1.25f;

        ImGui::SetNextItemWidth(IntEntryWidth);

        auto halfMoveCounter = static_cast<int>(position.position.halfmoveClock);

        if (ImGui::InputInt(HalfMovesLabel, &halfMoveCounter)) {
            position.position.halfmoveClock = static_cast<std::uint_least8_t>(
                std::clamp(halfMoveCounter, 0, 100));
            position.refresh_default_operations();
        }

        if (showTooltips)
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

        if (showTooltips)
            ImGui::SetItemTooltip(
                "The number of full turns in the game so far. This counter is incremented after each Black move.");
    }

    void render_fen_string(
        Position& position, string& errorMessage, const bool showTooltips)
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

        if (showTooltips)
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

    void render_epd_string(
        EPDPosition& position, string& errorMessage, const bool showTooltips)
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

        if (showTooltips)
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

    enum class EPDOperationType : std::uint_least8_t {
        Integer,
        String
    };

    struct StandardEPDOperation final {
        string           mnemonic;
        EPDOperationType type;
        string           tooltip;
    };

    [[nodiscard]] auto get_standard_epd_ops()
        -> std::span<const StandardEPDOperation>
    {
        // see https://www.chessprogramming.org/Extended_Position_Description

        // TODO: comment(s), draw offer/accept, resignation, v0 variation name (primary, also v1 though v9)
        // TODO: maybe a move type?
        // TODO: Maybe an IntegerNonNegative type?

        namespace ops = chess::notation::epd_ops;

        using Type = EPDOperationType;

        static const std::array operations {
            StandardEPDOperation { .mnemonic = string { ops::AnalysisCountDepth }, .type = Type::Integer, .tooltip = "Analysis count depth" },
            StandardEPDOperation { .mnemonic = string { ops::AnalysisCountNodes }, .type = Type::Integer, .tooltip = "Analysis count nodes" },
            StandardEPDOperation { .mnemonic = string { ops::AnalysisCountSeconds }, .type = Type::Integer, .tooltip = "Analysis count seconds" },
            StandardEPDOperation { .mnemonic = string { ops::AvoidMove }, .type = Type::String, .tooltip = "Avoid move(s)" },
            StandardEPDOperation { .mnemonic = string { ops::BestMove }, .type = Type::String, .tooltip = "Best move(s)" },
            StandardEPDOperation { .mnemonic = string { ops::CentipawnEvaluation }, .type = Type::Integer, .tooltip = "Centipawn evaluation" },
            StandardEPDOperation { .mnemonic = string { ops::DirectMate }, .type = Type::Integer, .tooltip = "Direct mate fullmove count" },
            StandardEPDOperation { .mnemonic = string { ops::EcoCode }, .type = Type::String, .tooltip = "Encyclopedia of Chess Openings opening code" },
            StandardEPDOperation { .mnemonic = string { ops::PositionID }, .type = Type::String, .tooltip = "Position identification" },
            StandardEPDOperation { .mnemonic = string { ops::NicCode }, .type = Type::String, .tooltip = "New In Chess opening code" },
            StandardEPDOperation { .mnemonic = string { ops::PredictedMove }, .type = Type::String, .tooltip = "Predicted move" },
            StandardEPDOperation { .mnemonic = string { ops::PredictedVariation }, .type = Type::String, .tooltip = "Predicted variation" },
            StandardEPDOperation { .mnemonic = string { ops::RepetitionCount }, .type = Type::Integer, .tooltip = "Repetition count" },
            StandardEPDOperation { .mnemonic = string { ops::SuppliedMove }, .type = Type::String, .tooltip = "Supplied move" }
        };

        return operations;
    }

    [[nodiscard]] auto is_standard_epd_mneumonic(
        const string_view name) -> bool
    {
        namespace ops = chess::notation::epd_ops;

        // these two aren't included in the list of standard ops we render,
        // but we also don't want to treat them as custom operations
        if (name == ops::FullmoveNumber or name == ops::HalfmoveClock)
            return true;

        return std::ranges::contains(
            get_standard_epd_ops(),
            name,
            &StandardEPDOperation::mnemonic);
    }

    void render_standard_epd_operations(
        EPDPosition& position, const bool showTooltips)
    {
        const ScopedGroup group;

        ImGui::SeparatorText("Standard operations");

        for (const auto& opMetadata : get_standard_epd_ops()) {
            auto inputBuf = [&position, &opMetadata] {
                const auto it = position.operations.find(opMetadata.mnemonic);

                if (it != position.operations.end())
                    return it->second;

                return string { };
            }();

            switch (opMetadata.type) {
                case EPDOperationType::Integer: {
                    auto value = util::strings::int_from_string<int>(inputBuf);

                    if (ImGui::InputInt(opMetadata.mnemonic.c_str(), &value, 1, 10, ImGuiInputTextFlags_CharsDecimal)) {
                        position.operations[opMetadata.mnemonic] = std::format("{}", value);
                    }
                    break;
                }

                default: [[fallthrough]];
                case EPDOperationType::String:
                    if (ImGui::InputText(opMetadata.mnemonic.c_str(), &inputBuf, InputTextFlags)) {
                        position.operations[opMetadata.mnemonic] = util::strings::trim(inputBuf);
                    }
            }

            if (showTooltips)
                ImGui::SetItemTooltip("%s", opMetadata.tooltip.c_str());
        }
    }

    void render_custom_epd_operations(
        EPDPosition& position, const bool showTooltips)
    {
        const ScopedGroup group;

        ImGui::SeparatorText("Custom operations");

        if (ImGui::Button("Add"))
            position.operations.emplace();

        if (showTooltips)
            ImGui::SetItemTooltip("Add a custom EPD operation");

        if (ImGui::BeginTable("Custom operations", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthFixed);

            for (auto& [name, value] : position.operations) {
                if (is_standard_epd_mneumonic(name))
                    continue;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                auto nameInput = name;

                if (ImGui::InputText("Name", &nameInput, InputTextFlags)) {
                    const auto prevValue = value;

                    position.operations.erase(name);

                    // TODO: warn if overwriting existing operation
                    position.operations[nameInput] = prevValue;

                    // inserting into the container might have invalidated iterators,
                    // need to stop iteration & redraw the whole table
                    ImGui::TableNextColumn();
                    ImGui::TableNextColumn();
                    break;
                }

                ImGui::TableNextColumn();

                ImGui::InputText("Value", &value, InputTextFlags);

                ImGui::TableNextColumn();

                if (ImGui::Button("Delete")) {
                    position.operations.erase(name);
                    break;
                }
            }

            ImGui::EndTable();
        }
    }

    void render_epd_editor(
        EPDPosition& position, string& errorMessage, const bool showTooltips)
    {
        if (ImGui::CollapsingHeader("EPD editor")) {
            render_epd_string(
                position, errorMessage, showTooltips);

            render_standard_epd_operations(
                position, showTooltips);

            render_custom_epd_operations(
                position, showTooltips);
        }
    }

    void render_engine_interop_buttons(
        EPDPosition& position, EngineWrapper& engine, const bool showTooltips)
    {
        const ScopedGroup group;

        auto& engineBase = static_cast<uci::EngineBase&>(engine);

        if (ImGui::Button("Send to engine")) {
            engineBase.abort_search();
            engineBase.wait();
            engine.set_position(position.position);
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Send position to engine (interrupts search if active)");

        ImGui::SameLine();

        if (ImGui::Button("Refresh from engine")) {
            position.position = engine.get_position();
            position.refresh_default_operations();

            if (const auto results = engine.get_results(); not results.empty())
                results.back().fill_standard_epd_operations(position);
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Reset to engine's current position");
    }
} // namespace

void render_board_editor(
    BoardEditorState& state,
    const bool        showTooltips,
    EngineWrapper&    engine)
{
    if (ImGui::Begin("Board editor")) {
        render_utility_buttons(
            state.position, showTooltips);

        render_side_to_move(
            state.position.position, showTooltips);

        render_castling_rights(
            state.position.position, showTooltips);

        render_ep_square(
            state.position.position, state.selectedEPSquare, showTooltips);

        render_move_counters(
            state.position, showTooltips);

        render_fen_string(
            state.position.position, state.fenParseError, showTooltips);

        UnformattedText(
            std::format("Zobrist key: {}", state.position.position.hash));

        render_epd_editor(
            state.position, state.epdParseError, showTooltips);

        ImGui::Separator();

        render_engine_interop_buttons(
            state.position, engine, showTooltips);

        // TODO: render chessboard
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

auto BoardEditorState::from_string(const string_view str) -> BoardEditorState
{
    const auto parsed = json::parse(str);

    BoardEditorState state;

    [[maybe_unused]] const auto result
        = from_epd(
            parsed.at(TAG_CURRENTBOARD).get<string_view>())
              .transform([&state](const EPDPosition& newPos) {
                  state.position = newPos;
                  return std::monostate { };
              })
              .transform_error(util::print_error);

    return state;
}

} // namespace ben_bot::gui
