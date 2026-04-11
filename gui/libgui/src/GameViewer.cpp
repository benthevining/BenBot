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
#include <filesystem>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libbenbot/engine/Engine.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/notation/PGN.hpp>
#include <libgui/ErrorPopup.hpp>
#include <libgui/FileDialogContext.hpp>
#include <libgui/GameViewer.hpp>
#include <libutil/Console.hpp>
#include <libutil/Files.hpp>
#include <libutil/Strings.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::gui {

using chess::notation::GameRecord;
using std::filesystem::path;
using std::string;
using std::string_view;

namespace {
    void show_pgn_load_dialog(
        GameRecord& game, FileDialogContext& chooser, ErrorPopup& error)
    {
        chooser.load_file(
            [&game, &error](const path& file) {
                [[maybe_unused]] const auto result
                    = util::files::load(file)
                          .and_then(chess::notation::from_pgn)
                          .transform([&game, &error](GameRecord&& loaded) {
                              game = std::move(loaded);
                              return error.set_success();
                          })
                          .transform_error([&error](string&& message) {
                              return error.set_error(std::move(message));
                          });
            });
    }

    void show_pgn_save_dialog(
        const GameRecord& game, FileDialogContext& chooser, ErrorPopup& error)
    {
        chooser.save_file(
            [&game, &error](const path& file) {
                [[maybe_unused]] const auto result
                    = util::files::overwrite(file, to_pgn(game))
                          .transform([&error] {
                              return error.set_success();
                          })
                          .transform_error([&error](string&& message) {
                              return error.set_error(std::move(message));
                          });
            });
    }

    void render_load_save_buttons(
        GameRecord& game, FileDialogContext& context, ErrorPopup& error, const bool showTooltips)
    {
        const ScopedGroup group;

        if (ImGui::Button("Load"))
            show_pgn_load_dialog(game, context, error);

        if (showTooltips)
            ImGui::SetItemTooltip("Load a game from a PGN file");

        ImGui::SameLine();

        if (ImGui::Button("Save"))
            show_pgn_save_dialog(game, context, error);

        if (showTooltips)
            ImGui::SetItemTooltip("Save game to a PGN file");

        error.render();
    }

    void render_raw_pgn_text(
        GameRecord& game, ErrorPopup& error)
    {
        if (not ImGui::CollapsingHeader("PGN text"))
            return;

        auto inputBuf = to_pgn(game);

        if (ImGui::InputTextMultiline("##PGN", &inputBuf)) {
            [[maybe_unused]] const auto result
                = chess::notation::from_pgn(inputBuf)
                      .transform([&game, &error](GameRecord&& parsed) {
                          game = std::move(parsed);
                          return error.set_success();
                      })
                      .transform_error([&error](string&& message) {
                          return error.set_error(std::move(message));
                      });
        }

        error.render();
    }

    void render_metadata_tags(
        GameRecord& game)
    {
        if (not ImGui::CollapsingHeader("Metadata"))
            return;

        const ScopedGroup group;

        // TODO: add/delete tags
        // TODO: special handling for 7-tag roster, Position/Setup keys

        for (auto& [key, value] : game.metadata) {
            ImGui::InputText(
                key.c_str(), &value, InputTextFlags);
        }
    }

    // TODO: highlight focused move
    // TODO: buttons for moves
    // TODO: variations
    void render_move_list(
        const GameRecord& game, const Engine& engine)
    {
        if (not ImGui::CollapsingHeader("Move list"))
            return;

        if (ImGui::BeginTable("Move list", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Move number");
            ImGui::TableSetupColumn("White");
            ImGui::TableSetupColumn("Black");

            ImGui::TableHeadersRow();

            const auto moveFormat = engine.get_pretty_print_move_format();

            auto position = game.get_starting_position();
            auto moveIdx  = 0uz;

            while (moveIdx < game.moves.moves.size()) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                UnformattedText(
                    std::format("{}.", position.fullMoveCounter));

                ImGui::TableNextColumn();

                // White move
                // handle case where game started with Black to move
                if (position.is_black_to_move()) {
                    assert(moveIdx == 0uz);
                } else {
                    const auto move = game.moves.moves.at(moveIdx++);

                    UnformattedText(
                        format_move(moveFormat, position, move.move));

                    position.make_move(move.move);

                    if (moveIdx >= game.moves.moves.size())
                        break;
                }

                // Black move
                ImGui::TableNextColumn();

                const auto move = game.moves.moves.at(moveIdx++);

                UnformattedText(
                    format_move(moveFormat, position, move.move));

                position.make_move(move.move);
            }

            ImGui::EndTable();
        }
    }

    void render_focused_move_info(
        const GameRecord::Move& move, const bool showTooltips)
    {
        // TODO: comment, NAGs
    }
} // namespace

void render_game_viewer(
    GameViewerState& state, const Engine& engine, const bool showTooltips)
{
    if (ImGui::Begin("Game")) {
        render_load_save_buttons(
            state.game, state.pgnLoadSave, state.pgnFileError, showTooltips);

        render_raw_pgn_text(
            state.game, state.pgnParseError);

        ImGui::Separator();

        render_metadata_tags(
            state.game);

        ImGui::Separator();

        render_move_list(
            state.game, engine);

        // TODO: forward/back buttons
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_PGN { "pgn" };
inline constexpr string_view TAG_FOCUSED_MOVE { "focused_move" };

auto GameViewerState::to_string() const -> string
{
    json data;

    data[TAG_PGN] = to_pgn(game);

    return data.dump();
}

auto GameViewerState::from_string(const string_view str) -> GameViewerState
{
    const auto parsed = json::parse(str);

    GameViewerState state;

    [[maybe_unused]] const auto result
        = chess::notation::from_pgn(
            parsed.at(TAG_PGN).get<string_view>())
              .transform([&state](GameRecord&& loaded) {
                  state.game = std::move(loaded);
                  return std::monostate { };
              })
              .transform_error(util::print_error);

    return state;
}

} // namespace ben_bot::gui
