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
#include <ratio>
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
        .value();
}

auto evaluation(
    const Score score) -> string
{
    return std::visit(
        util::Visitor {
            [](const Score::Centipawns centipawns) {
                return std::format(
                    "{:+}",
                    centipawns.value);
            },
            [](const Score::MateIn mate) {
                return std::format(
                    "#{}",
                    std::abs(mate.moves()));
            } },
        score.value);
}

namespace {
    template <typename Ratio, char Suffix, size_t Precision>
    [[nodiscard]] auto get_quantity_string(
        const size_t value) -> std::optional<string>
    {
        if (value >= Ratio::num) {
            const auto display = static_cast<float>(value) / static_cast<float>(Ratio::num);

            return std::format(
                "{:.{}f}{}",
                display, Precision, Suffix);
        }

        return std::nullopt;
    }

    // formats a value in the forms "100", "1k", "1M" depending on its magnitude
    template <size_t Precision>
    [[nodiscard]] auto format_quantity(
        const size_t value) -> string
    {
        return get_quantity_string<std::giga, 'B', Precision>(value)
            .or_else([value] { return get_quantity_string<std::mega, 'M', Precision>(value); })
            .or_else([value] { return get_quantity_string<std::kilo, 'k', Precision>(value); })
            .or_else([value] { return std::make_optional(std::format("{}", value)); })
            .value();
    }
} // namespace

auto nodes(
    const size_t num) -> string
{
    return format_quantity<2uz>(num);
}

auto nps(
    const size_t nodesPerSec) -> string
{
    return std::format(
        "{}/s",
        format_quantity<1uz>(nodesPerSec));
}

auto hashfull(
    const size_t permille) -> string
{
    return std::format(
        "{}%",
        permille / 10uz);
}

namespace {
    [[nodiscard, gnu::const]] auto to_percentage(
        const size_t stat, const size_t totalNodes) noexcept -> size_t
    {
        return static_cast<size_t>(
            std::round(
                (static_cast<double>(stat) / static_cast<double>(totalNodes)) * 100.));
    }
} // namespace

auto search_stat(
    const size_t stat, const size_t totalNodes) -> string
{
    return std::format(
        "{} ({}%)",
        format_quantity<1uz>(stat),
        to_percentage(stat, totalNodes));
}

} // namespace ben_bot::pretty_print
