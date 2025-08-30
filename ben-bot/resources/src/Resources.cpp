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
#include <chrono>
#include <cmrc/cmrc.hpp>
#include <ctime>
#include <format>
#include <iomanip>
#include <sstream>
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

string_view get_opening_book_pgn_text()
{
    return get_named_resource("book.pgn");
}

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

namespace {
    namespace chrono = std::chrono;

    [[nodiscard, gnu::const]] consteval chrono::year build_year()
    {
        return chrono::year {
            (__DATE__[7] - '0') * 1000
            + (__DATE__[8] - '0') * 100
            + (__DATE__[9] - '0') * 10
            + (__DATE__[10] - '0')
        };
    }

    [[nodiscard, gnu::const]] consteval chrono::month build_month()
    {
        return chrono::month {
            __DATE__[0] == 'J'   ? (__DATE__[1] == 'a' ? 1u
                                                       : (__DATE__[2] == 'n' ? 6u : 7u))
            : __DATE__[0] == 'F' ? 2
            : __DATE__[0] == 'M' ? (__DATE__[2] == 'r' ? 3u : 5u)
            : __DATE__[0] == 'A' ? (__DATE__[1] == 'p' ? 4u : 8u)
            : __DATE__[0] == 'S' ? 9u
            : __DATE__[0] == 'O' ? 10u
            : __DATE__[0] == 'N' ? 11u
            : __DATE__[0] == 'D' ? 12u
                                 : 0u
        };
    }

    [[nodiscard, gnu::const]] consteval chrono::day build_day()
    {
        return chrono::day {
            (__DATE__[4] == ' ') ? static_cast<unsigned>(__DATE__[5] - '0')
                                 : static_cast<unsigned>((__DATE__[4] - '0') * 10
                                                         + (__DATE__[5] - '0'))
        };
    }

    [[nodiscard, gnu::const]] consteval std::chrono::sys_days build_date()
    {
        return chrono::sys_days {
            chrono::year_month_day {
                build_year(), build_month(), build_day() }
        };
    }
} // namespace

std::string get_build_time()
{
    static constexpr auto timePoint = build_date();

    const auto timeT = chrono::system_clock::to_time_t(timePoint);

    if (const auto* utcTime = std::gmtime(&timeT)) {
        std::ostringstream stream;
        stream << std::put_time(utcTime, "%c %Z");
        return stream.str();
    }

    return {};
}

} // namespace ben_bot::resources
