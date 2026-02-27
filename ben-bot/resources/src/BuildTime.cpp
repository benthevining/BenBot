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
#include <charconv>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <ctime>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::resources {

namespace {
    using std::string_view;

    namespace chrono = std::chrono;

    // format is "MMM DD YYYY", e.g., "Feb 01 1994"
    inline constexpr string_view BUILD_DATE_STR { __DATE__ };

    // format is "HH:MM:SS", e.g., 14:30:15
    inline constexpr string_view BUILD_TIME_STR { __TIME__ };

    // NB. This is duplicated from libchess's string utility header, but I didn't
    // want to introduce a dependency on libchess just for this one function.
    [[nodiscard]] consteval auto int_from_string(const string_view text) noexcept -> size_t
    {
        auto value { 0uz };

        std::from_chars(
            text.data(), // NOLINT(bugprone-suspicious-stringview-data-usage)
            std::next(text.data(), static_cast<std::ptrdiff_t>(text.length())),
            value);

        return value;
    }

    [[nodiscard, gnu::const]] consteval auto build_year() noexcept -> chrono::year // NOLINT(bugprone-exception-escape)
    {
        // the year is the last 4 characters of the date string
        static constexpr auto yearStrLen = 4uz;

        static constexpr auto yearStr = BUILD_DATE_STR.substr(
            BUILD_DATE_STR.size() - yearStrLen, yearStrLen);

        return chrono::year {
            static_cast<int>(int_from_string(yearStr))
        };
    }

    [[nodiscard, gnu::const]] consteval auto build_month() noexcept -> chrono::month // NOLINT(bugprone-exception-escape)
    {
        switch (BUILD_DATE_STR.front()) {
            case 'F': return std::chrono::February;
            case 'S': return std::chrono::September;
            case 'O': return std::chrono::October;
            case 'N': return std::chrono::November;
            case 'D': return std::chrono::December;

            case 'J': {
                switch (BUILD_DATE_STR.at(1uz)) {
                    case 'a': return std::chrono::January;
                    case 'n': return std::chrono::June;
                    default : return std::chrono::July;
                }
            }

            case 'M': {
                if constexpr (BUILD_DATE_STR.at(2uz) == 'r')
                    return std::chrono::March;
                else
                    return std::chrono::May;
            }

            case 'A': {
                if constexpr (BUILD_DATE_STR.at(1uz) == 'p')
                    return std::chrono::April;
                else
                    return std::chrono::August;
            }

            default:
                std::unreachable();
        }
    }

    [[nodiscard, gnu::const]] consteval auto build_day() noexcept -> chrono::day // NOLINT(bugprone-exception-escape)
    {
        return chrono::day {
            int_from_string(BUILD_DATE_STR.substr(4uz, 2uz))
        };
    }

    [[nodiscard, gnu::const]] consteval auto build_time_of_day() noexcept -> chrono::minutes
    {
        static constexpr auto buildHour = chrono::hours {
            int_from_string(BUILD_TIME_STR.substr(0uz, 2uz))
        };

        static constexpr auto buildMinute = chrono::minutes {
            int_from_string(BUILD_TIME_STR.substr(3uz, 2uz))
        };

        return duration_cast<chrono::minutes>(buildHour) + buildMinute;
    }

    [[nodiscard, gnu::const]] consteval auto build_date() noexcept -> chrono::sys_time<chrono::minutes>
    {
        static constexpr auto date = chrono::sys_days {
            chrono::year_month_day {
                build_year(), build_month(), build_day() }
        };

        return time_point_cast<chrono::minutes>(date) + build_time_of_day();
    }

    [[nodiscard, gnu::cold]] auto to_utc_time(const std::time_t time) -> std::tm
    {
        std::tm ret { };

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
    // This defaults to false because the time output by this assumes that the binary
    // is running on the build machine (we have no way to know what timezone __TIME__
    // was recorded in), but it can be enabled as a sanity check when developing locally.
    static constexpr bool IncludeTimeOfDay = false;

    static constexpr auto timePoint = build_date();

    const auto utcTime = to_utc_time(chrono::system_clock::to_time_t(timePoint));

    std::string timeFormat { "%d %b %Y" };

    if constexpr (IncludeTimeOfDay) {
        timeFormat.append(" %X");
    }

    std::ostringstream stream;

    stream << std::put_time(&utcTime, timeFormat.data());

    return std::move(stream).str();
}

} // namespace ben_bot::resources
