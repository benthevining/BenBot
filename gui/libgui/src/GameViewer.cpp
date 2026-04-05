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
#include <concepts>
#include <filesystem>
#include <imgui.h>
#include <libchess/notation/PGN.hpp>
#include <libgui/GameViewer.hpp>
#include <libutil/Console.hpp>
#include <libutil/Files.hpp>
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
                                  .transform_error([](const string_view error) { return std::string { error }; });
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

    void render_move_list(
        const GameRecord& game, const bool showTooltips)
    {
        // TODO
    }

    void render_focused_move(
        const GameRecord::Move& move, const bool showTooltips)
    {
        // TODO: comment, NAGs, variations
    }
} // namespace

void render_game_viewer(
    GameViewerState& state, const bool showTooltips)
{
    if (ImGui::Begin("Game")) {
        render_load_save_buttons(
            state.game, showTooltips);

        render_move_list(
            state.game, showTooltips);

        // TODO: how to keep track of focused move?

        // TODO: forward/back buttons
    }

    ImGui::End();
}

using nlohmann::json;

inline constexpr string_view TAG_PGN { "pgn" };

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
