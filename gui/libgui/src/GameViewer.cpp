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
#include <array>
#include <cassert>
#include <concepts>
#include <filesystem>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <libbenbot/engine/Engine.hpp>
#include <libchess/notation/MoveFormats.hpp>
#include <libchess/notation/PGN.hpp>
#include <libgui/GameViewer.hpp>
#include <libutil/Console.hpp>
#include <libutil/Files.hpp>
#include <libutil/Strings.hpp>
#include <nfd.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using chess::notation::GameRecord;
using std::filesystem::path;
using std::string_view;

namespace {
    template <typename Func>
    concept FileCallback = std::regular_invocable<Func, const path&>;

    template <bool IsLoading>
    void show_file_dialog(FileCallback auto callback)
    {
        NFD::UniquePath outPath;

        const auto result = [&outPath]() noexcept {
            static constexpr std::array filters {
                nfdu8filteritem_t { "PGN", "pgn" }
            };

            if constexpr (IsLoading) {
                return OpenDialog(
                    outPath,
                    filters.data(), filters.size());
            } else {
                return SaveDialog(
                    outPath,
                    filters.data(), filters.size(),
                    nullptr, "game.pgn");
            }
        }();

        switch (result) {
            case NFD_OKAY:
                callback(path { outPath.get() });
                break;

            case NFD_CANCEL:
                std::println("Info: user canceled file selection dialog");
                break;

            case NFD_ERROR:
                std::println(
                    stderr, "Info: error with file selection dialog");
                break;

            default: break;
        }
    }

    void show_pgn_load_dialog(GameRecord& game)
    {
        show_file_dialog<true>(
            [&game](const path& file) {
                [[maybe_unused]] const auto result
                    = util::files::load(file)
                          .and_then([](const string_view text) {
                              return chess::notation::from_pgn(text)
                                  .transform_error(util::strings::to_owning_string);
                          })
                          .transform([&game](const GameRecord& loaded) {
                              game = loaded;
                              return std::monostate { };
                          })
                          .transform_error(util::print_error);
            });
    }

    void show_pgn_save_dialog(const GameRecord& game)
    {
        show_file_dialog<false>(
            [&game](const path& file) {
                [[maybe_unused]] const auto result
                    = util::files::overwrite(file, to_pgn(game))
                          .transform_error(util::print_error);
            });
    }

    void render_load_save_buttons(
        GameRecord& game, const bool showTooltips)
    {
        const ScopedGroup group;

        if (ImGui::Button("Load"))
            show_pgn_load_dialog(game);

        if (showTooltips)
            ImGui::SetItemTooltip("Load a game from a PGN file");

        ImGui::SameLine();

        if (ImGui::Button("Save"))
            show_pgn_save_dialog(game);

        if (showTooltips)
            ImGui::SetItemTooltip("Save game to a PGN file");
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
    void render_move_list(
        const GameRecord& game, const Engine& engine)
    {
        if (ImGui::BeginTable("Move list", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Move number");
            ImGui::TableSetupColumn("White");
            ImGui::TableSetupColumn("Black");

            ImGui::TableHeadersRow();

            const auto moveFormat = engine.get_pretty_print_move_format();

            auto position = game.startingPosition;
            auto moveIdx  = 0uz;

            while (moveIdx < game.moves.size()) {
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
                    const auto move = game.moves.at(moveIdx++);

                    UnformattedText(
                        format_move(moveFormat, position, move.move));

                    position.make_move(move.move);

                    if (moveIdx >= game.moves.size())
                        break;
                }

                // Black move
                ImGui::TableNextColumn();

                const auto move = game.moves.at(moveIdx++);

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
        // TODO: comment, NAGs, variations
    }
} // namespace

void render_game_viewer(
    GameViewerState& state, const Engine& engine, const bool showTooltips)
{
    if (ImGui::Begin("Game")) {
        render_load_save_buttons(
            state.game, showTooltips);

        render_metadata_tags(
            state.game);

        render_move_list(
            state.game, engine);

        // TODO: forward/back buttons
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_PGN { "pgn" };
inline constexpr string_view TAG_FOCUSED_MOVE { "focused_move" };

auto GameViewerState::to_string() const -> std::string
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
              .transform([&state](const GameRecord& loaded) {
                  state.game = loaded;
                  return std::monostate { };
              })
              .transform_error(util::print_error);

    return state;
}

} // namespace ben_bot::gui
