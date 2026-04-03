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

#include <chrono>
#include <cmath> // IWYU pragma: keep - for std::abs()
#include <format>
#include <libbenbot/search/PrettyPrinting.hpp>
#include <libutil/Chrono.hpp>
#include <libutil/Variant.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace ben_bot::pretty_print {

namespace {
    using std::chrono::milliseconds;
    using std::string;
    using std::string_view;

    inline constexpr string_view FRACTIONAL_DURATION_FMT { "{:.2%Q %q}" };
    inline constexpr string_view INTEGER_DURATION_FMT { "{:%Q %q}" };

    template <util::ChronoDuration Duration>
    [[nodiscard]] auto get_duration_string(
        const milliseconds duration) -> std::optional<string>
    {
        static constexpr auto msPerUnit = duration_cast<milliseconds>(Duration { 1uz });

        if (duration >= msPerUnit) {
            using FractionalDuration = util::FractionalDuration<Duration>;

            return std::format(
                FRACTIONAL_DURATION_FMT,
                duration_cast<FractionalDuration>(duration));
        }

        return std::nullopt;
    }
} // namespace

auto duration(
    const milliseconds duration) -> string
{
    // NB. it should be quite rare that a search will run for 1 day or more...
    return get_duration_string<std::chrono::hours>(duration)
        .or_else([duration] { return get_duration_string<std::chrono::minutes>(duration); })
        .or_else([duration] { return get_duration_string<std::chrono::seconds>(duration); })
        .or_else([duration] {
            return std::make_optional(std::format(INTEGER_DURATION_FMT, duration));
        })
        .value_or(std::string { });
}

auto evaluation(
    const Score score) -> string
{
    return std::visit(
        util::Visitor {
            [](const Score::Centipawns& centipawns) {
                return std::format(
                    "{:+}",
                    centipawns.value);
            },
            [](const Score::MateIn& mate) {
                return std::format(
                    "#{}",
                    std::abs(mate.moves()));
            } },
        score.value);
}

} // namespace ben_bot::pretty_print
