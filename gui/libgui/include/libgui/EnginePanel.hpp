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
    This file defines the engine panel state and rendering function.
    @ingroup libgui
 */

#pragma once

#include <libbenbot/search/Options.hpp>
#include <libgui/EngineWrapper.hpp>
#include <libgui/ErrorPopup.hpp>
#include <string>
#include <string_view>

namespace ben_bot::gui {

/** This struct encapsulates the state of the engine panel.
    @ingroup libgui
 */
struct EnginePanelState final {
    /** The engine itself. */
    EngineWrapper engine;

    /** The search options used in the UI editor. */
    search::Options searchOptions;

    /** Used for multichoice UCI options. */
    std::optional<std::string> selectedComboChoice;

    /** Error popup context used for parsing moves-to-search input. */
    ErrorPopup moveParseError { "Move parse error" };

    /** Serializes this state to a JSON string. */
    [[nodiscard]] auto to_string() const -> std::string;

    /** Loads a saved state from a JSON string.

        @note This method updates an existing object
        instead of returning a new state because the engine
        itself lives in here, and we don't want to mess with
        its lifetime unnecessarily.
     */
    void update_from_string(std::string_view str);
};

/** Renders an engine panel.
    This panel includes a UI for setting UCI and search options,
    and renders search output.

    @ingroup libgui
    @relates EnginePanelState
 */
void render_engine_panel(
    EnginePanelState& state, bool showTooltips);

} // namespace ben_bot::gui
