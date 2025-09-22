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
#include <ben-bot/Resources.hpp>
#include <cmrc/cmrc.hpp>
#include <format>
#include <string>
#include <string_view>

CMRC_DECLARE(ben_bot_resources_internal);

namespace ben_bot::resources {

using std::string_view;

namespace {
    [[nodiscard]] string_view get_named_resource(const string_view name)
    {
        const auto relPath = std::format("res/{}", name);

        const auto file = cmrc::ben_bot_resources_internal::get_filesystem()
                              .open(relPath);

        return string_view { file }; // NOLINT
    }
} // namespace

string_view get_bench_epd_text()
{
    return get_named_resource("bench.epd");
}

string_view get_ascii_logo()
{
    return get_named_resource("license_header.txt");
}

// the below functions are implemented this way because
// this is the only TU that includes BenBotConfig.hpp

string_view get_version_string()
{
    return config::VERSION_STRING;
}

string_view get_compiler_name()
{
    return config::COMPILER_NAME;
}

string_view get_compiler_version()
{
    return config::COMPILER_VERSION;
}

string_view get_system_name()
{
    return config::SYSTEM_NAME;
}

string_view get_build_config()
{
    return config::BUILD_CONFIG;
}

} // namespace ben_bot::resources
