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

#include <BenBotConfig.hpp>
#include <cmrc/cmrc.hpp>
#include <format>
#include <libbenbot/Resources.hpp>
#include <string>
#include <string_view>

CMRC_DECLARE(ben_bot_resources_internal);

namespace ben_bot::resources {

using std::string_view;

namespace {
    [[nodiscard]] auto get_named_resource(const string_view name) -> string_view
    {
        const auto relPath = std::format("res/{}", name);

        const auto file = cmrc::ben_bot_resources_internal::get_filesystem()
                              .open(relPath);

        return string_view { file }; // NOLINT
    }
} // namespace

auto get_bench_epd_text() -> string_view
{
    return get_named_resource("bench.epd");
}

auto get_ascii_logo() -> string_view
{
    return get_named_resource("license_header.txt");
}

// the below functions are implemented this way because
// this is the only TU that includes BenBotConfig.hpp

auto get_version_string() -> string_view
{
    return config::VERSION_STRING;
}

auto get_compiler_name() -> string_view
{
    return config::COMPILER_NAME;
}

auto get_compiler_version() -> string_view
{
    return config::COMPILER_VERSION;
}

auto get_system_name() -> string_view
{
    return config::SYSTEM_NAME;
}

auto get_build_config() -> string_view
{
    return config::BUILD_CONFIG;
}

} // namespace ben_bot::resources
