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
    This file defines the board editor state and rendering function.
    @ingroup libgui
 */

#pragma once

#include <libchess/board/Square.hpp>
#include <libchess/notation/EPD.hpp>
#include <libgui/ErrorPopup.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace ben_bot::gui {

struct EngineWrapper;

using chess::notation::EPDPosition;

/** This struct encapsulates the state of the board editor.
    @ingroup libgui
 */
struct BoardEditorState final {
    /** The current position. */
    EPDPosition position;

    /** Used for the en passant square combobox. */
    std::optional<chess::board::Square> selectedEPSquare;

    /** Error popup context used for parsing FEN string input. */
    ErrorPopup fenParseError { "FEN parse error" };

    /** Error popup context used for parsing EPD string input. */
    ErrorPopup epdParseError { "EPD parse error" };

    /** Serializes this state to a JSON string. */
    [[nodiscard]] auto to_string() const -> std::string;

    /** Restores a state from a saved JSON string. */
    [[nodiscard]] static auto from_string(std::string_view str) -> BoardEditorState;
};

/** Renders a board editor.

    @ingroup libgui
    @relates BoardEditorState
 */
void render_board_editor(
    BoardEditorState& state,
    bool              showTooltips,
    EngineWrapper&    engine);

} // namespace ben_bot::gui
