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
#include <cassert>
#include <chrono>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/uci/Options.hpp>
#include <libgui/EnginePanel.hpp>
#include <libutil/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
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
        for (auto* opt : engine.get_standard_uci_options())
            opt->reset_to_default_value();

        for (auto* opt : engine.get_custom_uci_options())
            opt->reset_to_default_value();
    }

    void render_uci_options(
        uci::EngineBase&       engine,
        std::optional<string>& selectedComboChoice,
        const bool             showTooltips)
    {
        if (ImGui::CollapsingHeader("UCI options")) {
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

    // returns true if input text changed
    [[nodiscard]] auto render_moves_to_search(
        search::Options& options,
        const MoveFormat moveFormat,
        const Position&  position,
        string&          errorMessage,
        const bool       showTooltips)
        -> bool
    {
        static constexpr auto ErrorPopupID { "Move parse error" };

        bool anyChanged { false };

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

            anyChanged = true;
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

        return anyChanged;
    }

    // returns true if any options were changed
    [[nodiscard]] auto render_search_options(
        search::Options& options,
        const MoveFormat moveFormat,
        const Position&  position,
        string&          moveParseError,
        const bool       showTooltips)
        -> bool
    {
        // TODO: handling of negative integer values, optionals

        bool anyChanged { false };

        if (ImGui::CollapsingHeader("Search options")) {
            auto depth = static_cast<int>(options.depth);

            if (ImGui::InputInt("Depth", &depth)) {
                options.depth = static_cast<size_t>(depth);
                anyChanged    = true;
            }

            if (showTooltips)
                ImGui::SetItemTooltip("Search depth, in plies");

            auto numMs = static_cast<int>(options.searchTime.value_or(std::chrono::milliseconds { 0 }).count());

            if (ImGui::InputInt("Time", &numMs)) {
                options.searchTime = std::chrono::milliseconds { numMs };
                anyChanged         = true;
            }

            if (showTooltips)
                ImGui::SetItemTooltip("Search time, in milliseconds");

            auto maxNodes = static_cast<int>(options.maxNodes);

            if (ImGui::InputInt("Nodes", &maxNodes)) {
                options.maxNodes = static_cast<size_t>(maxNodes);
                anyChanged       = true;
            }

            if (showTooltips)
                ImGui::SetItemTooltip("Maximum number of nodes to search");

            auto mateIn = static_cast<int>(options.mateIn.value_or(0uz));

            if (ImGui::InputInt("Mate in", &mateIn)) {
                options.mateIn = static_cast<size_t>(mateIn);
                anyChanged     = true;
            }

            if (showTooltips)
                ImGui::SetItemTooltip("Search for mate in X plies");

            anyChanged = render_moves_to_search(options, moveFormat, position, moveParseError, showTooltips)
                      || anyChanged;

            if (ImGui::Checkbox("Infinite", &options.infinite))
                anyChanged = true;

            if (showTooltips)
                ImGui::SetItemTooltip("Whether to search infinitely");

            if (ImGui::Button("Reset")) {
                options    = search::Options { };
                anyChanged = true;
            }

            if (showTooltips)
                ImGui::SetItemTooltip("Reset search options to defaults");
        }

        return anyChanged;
    }
} // namespace

void render_engine_panel(
    EnginePanelState& state, const bool showTooltips)
{
    if (ImGui::Begin("Engine")) {
        render_uci_options(state.engine, state.selectedComboChoice, showTooltips);

        ImGui::Separator();

        if (render_search_options(
                state.searchOptions, state.engine.get_move_format(), state.engine.get_position(), state.moveParseError, showTooltips)) {
            // TODO: probably shouldn't block here...
            state.engine.set_search_options(state.searchOptions);
        }
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
