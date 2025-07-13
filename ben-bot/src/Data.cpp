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

#include "Data.hpp"
#include "BenBotConfig.hpp"
#include <cmrc/cmrc.hpp>
#include <string>
#include <string_view>

CMRC_DECLARE(ben_bot_resources);

namespace ben_bot {

namespace {
    [[nodiscard]] string_view get_named_resource(const std::string& name)
    {
        const auto file = cmrc::ben_bot_resources::get_filesystem()
                              .open(name);

        return string_view { file }; // NOLINT
    }
} // namespace

string_view get_opening_book_pgn_text()
{
    return get_named_resource("book.pgn");
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

} // namespace ben_bot
