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
#include <libgui/ErrorPopup.hpp>
#include <libutil/Strings.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>

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

        if (ImGui::Button("Reset all"))
            reset_all_options(engine);

        ImGui::SetItemTooltip("Reset all options to their default values");

        auto debug = engine.is_debug_mode();

        if (ImGui::Checkbox("Debug mode", &debug))
            engine.set_debug_mode(debug);

        ImGui::SetItemTooltip("Enable engine debug mode");

        ImGui::SeparatorText("Standard options");

        for (auto* opt : engine.get_standard_uci_options())
            render_uci_option(*opt, selectedComboChoice, showTooltips);

        ImGui::SeparatorText("Custom options");

        for (auto* opt : engine.get_custom_uci_options())
            render_uci_option(*opt, selectedComboChoice, showTooltips);
    }

    using chess::game::Position;
    using chess::notation::MoveFormat;

    [[nodiscard]] auto format_moves(
        const chess::moves::MoveList& moves,
        const Engine&                 engine)
        -> string
    {
        const auto  format   = engine.get_pretty_print_move_format();
        const auto& position = engine.get_position();

        string text;

        for (const auto move : moves) {
            // Use the format_move() function directly instead of the engine's
            // pretty_print_move() function because the engine's function will
            // print piece types as UTF8 if that option is enabled. We always
            // want to bypass this behavior here so that we're always giving
            // the input text a default value that can be parsed successfully.
            text.append(format_move(
                format, position, move));

            text.append(1uz, ' ');
        }

        // remove trailing space
        if (not text.empty())
            text.pop_back();

        return text;
    }

    void render_moves_to_search(
        search::Options& options,
        const Engine&    engine,
        ErrorPopup&      errorPopup,
        const bool       showTooltips)
    {
        auto inputBuf = format_moves(options.movesToSearch, engine);

        if (ImGui::InputText("Moves to search", &inputBuf, InputTextFlags)) {
            options.movesToSearch.clear();

            const auto  format   = engine.get_pretty_print_move_format();
            const auto& position = engine.get_position();

            for (const auto word : util::strings::words_view(inputBuf)) {
                [[maybe_unused]] const auto result
                    = parse_move(format, position, word)
                          .transform([&options](const Move move) {
                              options.movesToSearch.emplace_back(move);
                              return std::monostate { };
                          })
                          .transform_error([&errorPopup](string&& message) {
                              errorPopup.set_error(std::move(message));
                              return std::monostate { };
                          });
            }
        }

        if (showTooltips)
            ImGui::SetItemTooltip("Search only the given moves");

        errorPopup.render();
    }

    void render_search_engine_interop_buttons(
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
        ErrorPopup&      moveParseError,
        Engine&          engine,
        const bool       showTooltips)
    {
        // TODO: handling of negative integer values, optionals

        if (not ImGui::CollapsingHeader("Search options"))
            return;

        render_search_engine_interop_buttons(options, engine, showTooltips);

        const ScopedGroup group;

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

        render_moves_to_search(
            options, engine, moveParseError, showTooltips);

        ImGui::Checkbox("Infinite", &options.infinite);

        if (showTooltips)
            ImGui::SetItemTooltip("Whether to search infinitely");
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

    [[nodiscard]] auto format_pv(
        const std::span<const Move> pv,
        const Engine&               engine) -> string
    {
        const auto format   = engine.get_pretty_print_move_format();
        auto       position = engine.get_position();

        string result;

        for (const auto move : pv) {
            result.append(
                format_move(format, position, move));

            result.append(1uz, ' ');

            position.make_move(move);
        }

        if (not result.empty())
            result.pop_back(); // remove trailing space

        return result;
    }

    void render_search_output(
        const EngineWrapper& engine,
        const bool           showTooltips)
    {
        if (not ImGui::CollapsingHeader("Search output"))
            return;

        if (ImGui::BeginTable("Search results", 11, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Depth", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Nodes", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("NPS", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Hashfull", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("TT hits", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Beta cutoffs", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("MDP cutoffs", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Static evals", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Evaluation", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("PV");

            ImGui::TableHeadersRow();

            for (const auto& result : engine.get_results()) {
                ImGui::TableNextRow();

                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        std::format("{} / {}", result.depth, result.qDepth));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Search depth / quiescence search depth, in plies");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::duration(result.duration));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Total search duration");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::nodes(result.nodesSearched));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Total number of nodes searched");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::nps(
                            result.to_libchess(false).get_nps()));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Nodes searched per second");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::hashfull(result.hashfull));

                    if (showTooltips)
                        ImGui::SetItemTooltip(
                            "Percentage of the transposition table that has been filled during the current search");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::search_stat(
                            result.transpositionTableHits, result.nodesSearched));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Number of transposition table hits");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::search_stat(
                            result.betaCutoffs, result.nodesSearched));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Number of nodes that were pruned due to beta cutoffs");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::search_stat(
                            result.mdpCutoffs, result.nodesSearched));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Number of nodes that were pruned due to mate distance pruning");
                }
                {
                    ImGui::TableNextColumn();
                    UnformattedText(
                        pretty_print::search_stat(
                            result.staticEvals, result.nodesSearched));

                    if (showTooltips)
                        ImGui::SetItemTooltip("Number of static analyses performed");
                }
                {
                    ImGui::TableNextColumn();

                    static constexpr auto GreenText = IM_COL32(0, 255, 0, 255);
                    static constexpr auto RedText   = IM_COL32(255, 0, 0, 255);
                    static constexpr auto GrayText  = IM_COL32(62, 62, 64, 255);

                    const auto score = result.score.to_libchess();

                    switch (score.get_type()) {
                        using enum pretty_print::Score::Type;

                        case Winning:
                            ImGui::PushStyleColor(ImGuiCol_Text, GreenText);
                            break;

                        case Losing:
                            ImGui::PushStyleColor(ImGuiCol_Text, RedText);
                            break;

                        default: [[fallthrough]];
                        case Equal:
                            ImGui::PushStyleColor(ImGuiCol_Text, GrayText);
                            break;
                    }

                    UnformattedText(
                        pretty_print::evaluation(score));

                    ImGui::PopStyleColor();

                    if (showTooltips)
                        ImGui::SetItemTooltip("Evaluation based on the best continuation");
                }
                {
                    ImGui::TableNextColumn();

                    UnformattedText(
                        format_pv(result.pv, engine));

                    if (showTooltips)
                        ImGui::SetItemTooltip("The best continuation found");
                }
            }

            ImGui::EndTable();
        }
    }
} // namespace

void render_engine_panel(
    EnginePanelState& state, const bool showTooltips)
{
    if (ImGui::Begin("Engine")) {
        render_uci_options(
            state.engine, state.selectedComboChoice, showTooltips);

        ImGui::Separator();

        render_search_options(
            state.searchOptions, state.moveParseError, state.engine, showTooltips);

        ImGui::Separator();

        render_start_stop_button(
            state.engine, state.searchOptions, showTooltips);

        ImGui::SameLine();

        if (ImGui::Button("New game"))
            state.engine.handle_command("ucinewgame");

        if (showTooltips)
            ImGui::SetItemTooltip("Send a ucinewgame command to the engine");

        render_search_output(
            state.engine, showTooltips);

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
