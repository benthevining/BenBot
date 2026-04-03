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
#include <cassert>
#include <chrono>
#include <format>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Options.hpp>
#include <libbenbot/search/PrettyPrinting.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/Options.hpp>
#include <libgui/EnginePanel.hpp>
#include <libutil/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using std::string;
using std::string_view;

namespace {
    void render_uci_option(
        uci::Option&           opt,
        std::optional<string>& selectedComboChoice,
        const bool             showTooltips)
    {
        const string name { opt.get_name() };
        const string help { opt.get_help() };

        if (auto* option = dynamic_cast<uci::BoolOption*>(&opt)) {
            auto value = option->get_value();

            if (ImGui::Checkbox(name.c_str(), &value))
                option->set_value(value);

            if (showTooltips)
                ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        if (auto* option = dynamic_cast<uci::IntOption*>(&opt)) {
            auto value = option->get_value();

            if (ImGui::InputInt(name.c_str(), &value))
                option->set_value(value);

            if (showTooltips)
                ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        if (auto* option = dynamic_cast<uci::StringOption*>(&opt)) {
            string value { option->get_value() };

            if (ImGui::InputText(name.c_str(), &value, InputTextFlags))
                option->set_value(value);

            if (showTooltips)
                ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        if (auto* action = dynamic_cast<uci::Action*>(&opt)) {
            if (ImGui::Button(name.c_str()))
                action->handle_setvalue({ });

            if (showTooltips)
                ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        auto& option = dynamic_cast<uci::ComboOption&>(opt);

        const string value { option.get_value() };

        if (ImGui::BeginCombo(name.c_str(), value.c_str())) {
            for (const auto& choice : option.get_possible_values()) {
                const bool isSelected = selectedComboChoice.has_value() and choice == *selectedComboChoice;

                if (ImGui::Selectable(choice.c_str(), isSelected)) {
                    option.set_value(choice);
                    selectedComboChoice = choice;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        } else {
            selectedComboChoice = value;
        }

        if (showTooltips)
            ImGui::SetItemTooltip("%s", help.c_str());
    }

    void reset_all_options(uci::EngineBase& engine)
    {
        std::ranges::for_each(
            std::views::join(std::array {
                engine.get_standard_uci_options(),
                engine.get_custom_uci_options() }),
            &uci::Option::reset_to_default_value);
    }

    void render_uci_options(
        uci::EngineBase&       engine,
        std::optional<string>& selectedComboChoice,
        const bool             showTooltips)
    {
        if (not ImGui::CollapsingHeader("UCI options"))
            return;

        const ScopedGroup group;

        ImGui::SeparatorText("Standard options");

        for (auto* opt : engine.get_standard_uci_options())
            render_uci_option(*opt, selectedComboChoice, showTooltips);

        ImGui::SeparatorText("Custom options");

        for (auto* opt : engine.get_custom_uci_options())
            render_uci_option(*opt, selectedComboChoice, showTooltips);

        ImGui::Separator();

        if (ImGui::Button("Reset all"))
            reset_all_options(engine);

        ImGui::SetItemTooltip("Reset all options to their default values");

        auto debug = engine.is_debug_mode();

        if (ImGui::Checkbox("Debug mode", &debug))
            engine.set_debug_mode(debug);

        ImGui::SetItemTooltip("Enable engine debug mode");
    }

    using chess::game::Position;
    using chess::notation::MoveFormat;

    [[nodiscard]] auto format_moves(
        const chess::moves::MoveList& moves,
        const MoveFormat              moveFormat,
        const Position&               position)
        -> string
    {
        string text;

        for (const auto move : moves) {
            text.append(format_move(moveFormat, position, move));
            text.append(1uz, ' ');
        }

        // remove trailing space
        if (not text.empty())
            text.pop_back();

        return text;
    }

    void render_moves_to_search(
        search::Options& options,
        const MoveFormat moveFormat,
        const Position&  position,
        string&          errorMessage,
        const bool       showTooltips)
    {
        static constexpr auto ErrorPopupID { "Move parse error" };

        auto inputBuf = format_moves(options.movesToSearch, moveFormat, position);

        if (ImGui::InputText("Moves to search", &inputBuf, InputTextFlags)) {
            options.movesToSearch.clear();

            for (const auto word : util::strings::words_view(inputBuf)) {
                [[maybe_unused]] const auto result
                    = parse_move(moveFormat, position, word)
                          .transform([&options](const Move move) {
                              options.movesToSearch.emplace_back(move);
                              return std::monostate { };
                          })
                          .transform_error([&errorMessage]([[maybe_unused]] const string_view message) {
                              assert(not message.empty());
                              errorMessage = message;
                              ImGui::OpenPopup(ErrorPopupID, ImGuiPopupFlags_NoReopen);
                              return std::monostate { };
                          });
            }
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Search only the given moves");

        if (ImGui::BeginPopupModal(ErrorPopupID, nullptr, PopupFlags)) {
            UnformattedText(errorMessage);

            if (ImGui::Button("OK", { 120.f, 0.f })) {
                ImGui::CloseCurrentPopup();
                errorMessage.clear();
            }

            ImGui::EndPopup();
        }
    }

    void render_utility_buttons(
        search::Options& options,
        Engine&          engine,
        const bool       showTooltips)
    {
        const ScopedGroup group;

        if (ImGui::Button("Reset"))
            options = search::Options { };

        if (showTooltips)
            ImGui::SetItemTooltip("Reset search options to defaults");

        ImGui::SameLine();

        if (ImGui::Button("Send to engine"))
            engine.set_search_options(options);

        if (showTooltips)
            ImGui::SetItemTooltip("Send search options to engine (interrupts search if active)");

        ImGui::SameLine();

        if (ImGui::Button("Refresh from engine"))
            options = engine.get_search_options();

        if (showTooltips)
            ImGui::SetItemTooltip("Reset to engine's current search options");
    }

    void render_search_options(
        search::Options& options,
        string&          moveParseError,
        Engine&          engine,
        const bool       showTooltips)
    {
        // TODO: handling of negative integer values, optionals

        if (not ImGui::CollapsingHeader("Search options"))
            return;

        const ScopedGroup group;

        const auto& position   = engine.get_position();
        const auto  moveFormat = engine.get_move_format();

        auto depth = static_cast<int>(options.depth);

        if (ImGui::InputInt("Depth", &depth))
            options.depth = static_cast<size_t>(depth);

        if (showTooltips)
            ImGui::SetItemTooltip("Search depth, in plies");

        auto numMs = static_cast<int>(options.searchTime.value_or(std::chrono::milliseconds { 0 }).count());

        if (ImGui::InputInt("Time", &numMs))
            options.searchTime = std::chrono::milliseconds { numMs };

        if (showTooltips)
            ImGui::SetItemTooltip("Search time, in milliseconds");

        auto maxNodes = static_cast<int>(options.maxNodes);

        if (ImGui::InputInt("Nodes", &maxNodes))
            options.maxNodes = static_cast<size_t>(maxNodes);

        if (showTooltips)
            ImGui::SetItemTooltip("Maximum number of nodes to search");

        auto mateIn = static_cast<int>(options.mateIn.value_or(0uz));

        if (ImGui::InputInt("Mate in", &mateIn))
            options.mateIn = static_cast<size_t>(mateIn);

        if (showTooltips)
            ImGui::SetItemTooltip("Search for mate in X plies");

        render_moves_to_search(options, moveFormat, position, moveParseError, showTooltips);

        ImGui::Checkbox("Infinite", &options.infinite);

        if (showTooltips)
            ImGui::SetItemTooltip("Whether to search infinitely");

        ImGui::Separator();

        render_utility_buttons(options, engine, showTooltips);
    }

    void render_start_stop_button(
        Engine&                engine,
        const search::Options& options,
        const bool             showTooltips)
    {
        auto& engineBase = static_cast<uci::EngineBase&>(engine);

        if (engineBase.is_searching()) {
            if (ImGui::Button("Stop"))
                engineBase.abort_search();

            if (showTooltips)
                ImGui::SetItemTooltip("Abort search");

            return;
        }

        if (ImGui::Button("Start"))
            engine.go(options);

        if (showTooltips)
            ImGui::SetItemTooltip("Start searching");
    }

    void render_search_output(
        const std::span<const search::Result> results,
        const bool                            showTooltips)
    {
        if (not ImGui::CollapsingHeader("Search output"))
            return;

        if (ImGui::BeginTable("Search results", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Depth");
            ImGui::TableSetupColumn("Duration");
            ImGui::TableSetupColumn("Evaluation");

            ImGui::TableHeadersRow();

            for (const auto& result : results) {
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                UnformattedText(
                    std::format("{} / {}", result.depth, result.qDepth));

                if (showTooltips)
                    ImGui::SetItemTooltip("Search depth / quiescence search depth, in plies");

                ImGui::TableNextColumn();
                UnformattedText(
                    pretty_print::duration(result.duration));

                if (showTooltips)
                    ImGui::SetItemTooltip("Total search duration so far");

                ImGui::TableNextColumn();
                UnformattedText(
                    pretty_print::evaluation(
                        result.score.to_libchess()));

                if (showTooltips)
                    ImGui::SetItemTooltip("Evaluation based on the best continuation");
            }

            ImGui::EndTable();
        }
    }
} // namespace

void render_engine_panel(
    EnginePanelState& state, const bool showTooltips)
{
    if (ImGui::Begin("Engine")) {
        render_uci_options(state.engine, state.selectedComboChoice, showTooltips);

        ImGui::Separator();

        render_search_options(
            state.searchOptions, state.moveParseError, state.engine, showTooltips);

        ImGui::Separator();

        render_start_stop_button(
            state.engine, state.searchOptions, showTooltips);

        render_search_output(
            state.engine.get_results(), showTooltips);

        // TODO: render current position as chessboard?
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_ENGINE { "engine" };
inline constexpr string_view TAG_OPTIONS { "search_options" };

auto EnginePanelState::to_string() const -> string
{
    json data;

    data[TAG_ENGINE]  = engine.state_to_string();
    data[TAG_OPTIONS] = searchOptions.to_string();

    return data.dump();
}

void EnginePanelState::update_from_string(const string_view str)
{
    const auto parsed = json::parse(str);

    engine.restore_state_from_string(
        parsed.at(TAG_ENGINE).get<string_view>());

    searchOptions = search::Options::from_string(
        parsed.at(TAG_OPTIONS).get<string_view>(),
        engine.get_position());
}

} // namespace ben_bot::gui
