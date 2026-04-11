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

/** @file
    This file defines the PGN game viewer state and rendering function.
    @ingroup libgui
 */

#pragma once

#include <libchess/notation/PGN.hpp>
#include <libgui/ErrorPopup.hpp>
#include <libgui/FileDialogContext.hpp>
#include <string>
#include <string_view>

namespace ben_bot {
class Engine;
} // namespace ben_bot

namespace ben_bot::gui {

/** This struct encapsulates the state of the PGN game viewer.
    @ingroup libgui
 */
struct GameViewerState final {
    /** The current PGN game. */
    chess::notation::GameRecord game;

    /** A file dialog context for loading and saving the current game to/from PGN files. */
    FileDialogContext pgnLoadSave {
        "Game.pgn",
        { { "PGN", "pgn" } }
    };

    /** Error popup context for parsing PGN text input. */
    ErrorPopup pgnParseError { "PGN parse error" };

    /** Error popup context for loading/saving PGN files. */
    ErrorPopup pgnFileError { "PGN file error" };

    /** Serializes this state to a JSON string. */
    [[nodiscard]] auto to_string() const -> std::string;

    /** Restores a game viewer state from a JSON string. */
    [[nodiscard]] static auto from_string(std::string_view str) -> GameViewerState;
};

/** Renders a PGN game viewer.

    @ingroup libgui
    @relates GameViewerState
 */
void render_game_viewer(
    GameViewerState& state, const Engine& engine, const Position& boardEditorPos, bool showTooltips);

} // namespace ben_bot::gui
