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
#include <libchess/notation/PGN.hpp>
#include <libgui/GameViewer.hpp>
#include <libutil/Console.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace ben_bot::gui {

using std::string_view;

void render_game_viewer(
    GameViewerState& state, const bool showTooltips)
{
    if (ImGui::Begin("Game")) {
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
              .transform([&state](const chess::notation::GameRecord& loaded) {
                  state.game = loaded;
                  return std::monostate { };
              })
              .transform_error(util::print_error);

    return state;
}

} // namespace ben_bot::gui
