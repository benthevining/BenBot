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

#include <imgui.h>
#include <imgui_stdlib.h>
#include <libbenbot/engine/Engine.hpp>
#include <libchess/uci/Options.hpp>
#include <libgui/EnginePanel.hpp>
#include <libutil/Variant.hpp>
#include <optional>
#include <string>

namespace ben_bot::gui {

using std::string;

inline constexpr auto InputTextFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;

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
        static constexpr auto CollapsibleFlags = ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_Framed;

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
        }
    }
} // namespace

void render_engine_panel(EnginePanelState& state)
{
    if (ImGui::Begin("Engine")) {
        render_uci_options(state.engine, state.selectedComboChoice);
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
