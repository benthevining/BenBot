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

/** @defgroup benbot_gui_data GUI resources
    Binary resources needed by libgui.
    @ingroup libgui
 */

/** @file
    This file defines functions for accessing data embedded into the ImGUI application.
    @ingroup benbot_gui_data
 */

#pragma once

#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <string_view>

/** This namespace contains resource data embedded into the ImGUI application.
    @ingroup benbot_gui_data
 */
namespace ben_bot::gui::resources {

using PieceType = chess::pieces::Type;
using chess::pieces::Color;
using std::string_view;

/** @ingroup benbot_gui_data
    @{
 */

/** INI data for the default UI layout. */
[[nodiscard, gnu::const]] auto get_default_imgui_ini_data() -> string_view;

/** JSON data for the default app state. */
[[nodiscard, gnu::const]] auto get_default_app_state() -> string_view;

/** Returns a piece sprite, as an SVG. */
[[nodiscard, gnu::const]] auto get_piece_sprite(
    PieceType type, Color color) -> string_view;

/** Returns the application icon, as a PNG.

    @warning The returned data may be empty if the application was built
    on a host machine without the ``inkscape`` tool present.
 */
[[nodiscard, gnu::const]] auto get_app_icon() -> string_view;

/** @} */

} // namespace ben_bot::gui::resources
