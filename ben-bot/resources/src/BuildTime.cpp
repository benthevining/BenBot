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

#include <ben-bot/Resources.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::resources {

namespace {
    constexpr std::string_view BUILD_DATE_STR { __DATE__ };

    namespace chrono = std::chrono;

    [[nodiscard, gnu::const]] consteval chrono::year build_year() noexcept
    {
        return chrono::year {
            ((BUILD_DATE_STR[7] - '0') * 1000)
            + ((BUILD_DATE_STR[8] - '0') * 100)
            + ((BUILD_DATE_STR[9] - '0') * 10)
            + (BUILD_DATE_STR[10] - '0')
        };
    }

    [[nodiscard, gnu::const]] consteval chrono::month build_month() noexcept
    {
        static constexpr auto monthNum = [] {
            switch (BUILD_DATE_STR.front()) {
                case 'J': {
                    if constexpr (BUILD_DATE_STR[1] == 'a')
                        return 1u;
                    else
                        return BUILD_DATE_STR[2] == 'n' ? 6u : 7u;
                }

                case 'F': return 2u;
                case 'S': return 9u;
                case 'O': return 10u;
                case 'N': return 11u;
                case 'D': return 12u;
                case 'M': return BUILD_DATE_STR[2] == 'r' ? 3u : 5u;
                case 'A': return BUILD_DATE_STR[1] == 'p' ? 4u : 8u;

                default:
                    std::unreachable();
            }
        }();

        return chrono::month { monthNum };
    }

    [[nodiscard, gnu::const]] consteval chrono::day build_day() noexcept
    {
        static constexpr auto dayNum = [] {
            if constexpr (BUILD_DATE_STR[4] == ' ')
                return static_cast<unsigned>(BUILD_DATE_STR[5] - '0');
            else
                return static_cast<unsigned>(((BUILD_DATE_STR[4] - '0') * 10) + (BUILD_DATE_STR[5] - '0'));
        }();

        return chrono::day { dayNum };
    }

    [[nodiscard, gnu::const]] consteval std::chrono::sys_days build_date() noexcept
    {
        return chrono::sys_days {
            chrono::year_month_day {
                build_year(), build_month(), build_day() }
        };
    }

    [[nodiscard]] std::tm to_utc_time(const std::time_t time)
    {
        std::tm ret {};

        // this is written this way to avoid using the thread-unsafe function gmtime()
        // gmtime_s() doesn't appear to be present on MacOS, and Windows's argument order
        // is reversed from the standardized version, so the if'def is necessary here
#ifdef _WIN32
        gmtime_s(&ret, &time);
#else
        gmtime_r(&time, &ret);
#endif

        return ret;
    }
} // namespace

std::string get_build_time()
{
    static constexpr auto timePoint = build_date();

    const auto utcTime = to_utc_time(chrono::system_clock::to_time_t(timePoint));

    std::ostringstream stream;

    stream << std::put_time(&utcTime, "%c %Z");

    return stream.str();
}

} // namespace ben_bot::resources
