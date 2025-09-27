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

    [[nodiscard, gnu::const]] consteval auto build_year() noexcept -> chrono::year
    {
        return chrono::year {
            ((BUILD_DATE_STR[7] - '0') * 1000)
            + ((BUILD_DATE_STR[8] - '0') * 100)
            + ((BUILD_DATE_STR[9] - '0') * 10)
            + (BUILD_DATE_STR[10] - '0')
        };
    }

    [[nodiscard, gnu::const]] consteval auto build_month() noexcept -> chrono::month
    {
        switch (BUILD_DATE_STR.front()) {
            case 'F': return std::chrono::February;
            case 'S': return std::chrono::September;
            case 'O': return std::chrono::October;
            case 'N': return std::chrono::November;
            case 'D': return std::chrono::December;

            case 'J': {
                switch (BUILD_DATE_STR[1]) {
                    case 'a': return std::chrono::January;
                    case 'n': return std::chrono::June;
                    default : return std::chrono::July;
                }
            }

            case 'M': {
                if constexpr (BUILD_DATE_STR[2] == 'r')
                    return std::chrono::March;
                else
                    return std::chrono::May;
            }

            case 'A': {
                if constexpr (BUILD_DATE_STR[1] == 'p')
                    return std::chrono::April;
                else
                    return std::chrono::August;
            }

            default:
                std::unreachable();
        }
    }

    [[nodiscard, gnu::const]] consteval auto build_day() noexcept -> chrono::day
    {
        static constexpr auto dayNum = [] {
            if constexpr (BUILD_DATE_STR[4] == ' ')
                return static_cast<unsigned>(BUILD_DATE_STR[5] - '0');
            else
                return static_cast<unsigned>(((BUILD_DATE_STR[4] - '0') * 10) + (BUILD_DATE_STR[5] - '0'));
        }();

        return chrono::day { dayNum };
    }

    [[nodiscard, gnu::const]] consteval auto build_date() noexcept -> std::chrono::sys_days
    {
        return chrono::sys_days {
            chrono::year_month_day {
                build_year(), build_month(), build_day() }
        };
    }

    [[nodiscard, gnu::cold]] auto to_utc_time(const std::time_t time) -> std::tm
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

auto get_build_time() -> std::string
{
    static constexpr auto timePoint = build_date();

    const auto utcTime = to_utc_time(chrono::system_clock::to_time_t(timePoint));

    std::ostringstream stream;

    stream << std::put_time(&utcTime, "%c %Z");

    return stream.str();
}

} // namespace ben_bot::resources
