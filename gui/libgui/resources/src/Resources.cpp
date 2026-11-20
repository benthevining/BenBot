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

#include <cmrc/cmrc.hpp>
#include <format>
#include <libgui/Resources.hpp>
#include <magic_enum/magic_enum.hpp>
#include <string_view>
#include <system_error>

CMRC_DECLARE(libgui_resources);

namespace ben_bot::gui::resources {

using std::string_view;

namespace {
    [[nodiscard]] auto get_named_resource(
        const string_view name) -> string_view
    {
        return string_view {
            cmrc::libgui_resources::get_filesystem()
                .open(std::format("res/{}", name))
        };
    }
} // namespace

auto get_default_imgui_ini_data() -> string_view
{
    return get_named_resource("default_layout.ini");
}

auto get_default_app_state() -> string_view
{
    return get_named_resource("default_state.json");
}

auto get_app_icon() -> string_view
try {
    return get_named_resource("icon.png");
} catch ([[maybe_unused]] const std::system_error& error) {
    return { };
}

auto get_piece_sprite(
    const PieceType type, const Color color) -> string_view
{
    using magic_enum::enum_name;

    return get_named_resource(
        std::format("pieces/{}-{}.svg",
            enum_name(type), enum_name(color)));
}

} // namespace ben_bot::gui::resources
