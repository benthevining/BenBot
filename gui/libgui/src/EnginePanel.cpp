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

#include <chrono>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/uci/Options.hpp>
#include <libgui/EnginePanel.hpp>
#include <libutil/Variant.hpp>
#include <optional>
#include <string>

namespace ben_bot::gui {

using std::string;

inline constexpr auto InputTextFlags   = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
inline constexpr auto CollapsibleFlags = ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_Framed;

namespace {
    void render_uci_option(uci::Option& opt, std::optional<string>& selectedComboChoice)
    {
        const string name { opt.get_name() };
        const string help { opt.get_help() };

        if (auto* option = dynamic_cast<uci::BoolOption*>(&opt)) {
            auto value = option->get_value();

            if (ImGui::Checkbox(name.c_str(), &value))
                option->set_value(value);

            ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        if (auto* option = dynamic_cast<uci::IntOption*>(&opt)) {
            auto value = option->get_value();

            if (ImGui::InputInt(name.c_str(), &value))
                option->set_value(value);

            ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        if (auto* option = dynamic_cast<uci::StringOption*>(&opt)) {
            string value { option->get_value() };

            if (ImGui::InputText(name.c_str(), &value, InputTextFlags))
                option->set_value(value);

            ImGui::SetItemTooltip("%s", help.c_str());
            return;
        }

        if (auto* action = dynamic_cast<uci::Action*>(&opt)) {
            if (ImGui::Button(name.c_str()))
                action->handle_setvalue({ });

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

        ImGui::SetItemTooltip("%s", help.c_str());
    }

    // TODO: give option base class a reset() method
    void reset_all_options(uci::EngineBase& engine)
    {
        auto reset_option = [](uci::Option& opt) {
            if (not opt.has_value())
                return;

            std::visit(util::Visitor {
                           [&opt](const bool value) { dynamic_cast<uci::BoolOption&>(opt).set_value(value); },
                           [&opt](const int value) { dynamic_cast<uci::IntOption&>(opt).set_value(value); },
                           [&opt](const std::string_view value) {
                               if (auto* comboOpt = dynamic_cast<uci::ComboOption*>(&opt))
                                   comboOpt->set_value(value);
                               else
                                   dynamic_cast<uci::StringOption&>(opt).set_value(value);
                           } },
                opt.get_default_value_variant());
        };

        for (auto* opt : engine.get_standard_uci_options())
            reset_option(*opt);

        for (auto* opt : engine.get_custom_uci_options())
            reset_option(*opt);
    }

    void render_uci_options(uci::EngineBase& engine, std::optional<string>& selectedComboChoice)
    {
        if (ImGui::CollapsingHeader("UCI options", CollapsibleFlags)) {
            ImGui::SeparatorText("Standard options");

            for (auto* opt : engine.get_standard_uci_options())
                render_uci_option(*opt, selectedComboChoice);

            ImGui::SeparatorText("Custom options");

            for (auto* opt : engine.get_custom_uci_options())
                render_uci_option(*opt, selectedComboChoice);

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

    // returns true if any options were changed
    [[nodiscard]] auto render_search_options(search::Options& options) -> bool
    {
        // TODO: handling of negative integer values, optionals
        // TODO: movesToSearch, mateIn

        bool anyChanged { false };

        if (ImGui::CollapsingHeader("Search options", CollapsibleFlags)) {
            auto depth = static_cast<int>(options.depth);

            if (ImGui::InputInt("Depth", &depth)) {
                options.depth = static_cast<size_t>(depth);
                anyChanged    = true;
            }

            ImGui::SetItemTooltip("Search depth, in plies");

            auto numMs = static_cast<int>(options.searchTime.value_or(std::chrono::milliseconds { 0 }).count());

            if (ImGui::InputInt("Time", &numMs)) {
                options.searchTime = std::chrono::milliseconds { numMs };
                anyChanged         = true;
            }

            ImGui::SetItemTooltip("Search time, in milliseconds");

            auto maxNodes = static_cast<int>(options.maxNodes);

            if (ImGui::InputInt("Nodes", &maxNodes)) {
                options.maxNodes = static_cast<size_t>(maxNodes);
                anyChanged       = true;
            }

            ImGui::SetItemTooltip("Maximum number of nodes to search");

            if (ImGui::Checkbox("Infinite", &options.infinite))
                anyChanged = true;

            ImGui::SetItemTooltip("Whether to search infinitely");
        }

        return anyChanged;
    }
} // namespace

void render_engine_panel(EnginePanelState& state)
{
    if (ImGui::Begin("Engine")) {
        render_uci_options(state.engine, state.selectedComboChoice);

        if (render_search_options(state.searchOptions))
            state.engine.set_search_options(state.searchOptions);
    }

    ImGui::End();
}

auto EnginePanelState::to_string() const -> string
{
    return engine.state_to_string();
}

void EnginePanelState::update_from_string(std::string_view str)
{
    engine.restore_state_from_string(str);
}

} // namespace ben_bot::gui
