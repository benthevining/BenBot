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
        .value_or(std::string { });
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
    [[nodiscard]] auto get_nodes_string(
        const size_t nodes) -> std::optional<string>
    {
        if (nodes >= Ratio::num) {
            const auto display = static_cast<float>(nodes) / static_cast<float>(Ratio::num);

            return std::format(
                "{:.{}f}{}",
                display, Precision, Suffix);
        }

        return std::nullopt;
    }

    template <size_t Precision>
    [[nodiscard]] auto format_nodes(
        const size_t nodes) -> string
    {
        return get_nodes_string<std::mega, 'M', Precision>(nodes)
            .or_else([nodes] { return get_nodes_string<std::kilo, 'k', Precision>(nodes); })
            .or_else([nodes] { return std::make_optional(std::format("{}", nodes)); })
            .value_or(std::string { });
    }
} // namespace

auto nodes(
    const size_t num) -> string
{
    return format_nodes<2uz>(num);
}

auto nps(
    const size_t nodesPerSec) -> string
{
    return std::format(
        "{}/s",
        format_nodes<1uz>(nodesPerSec)); // use this function for its transformation of the value to a k/M representation
}

auto hashfull(
    const size_t permille) -> string
{
    return std::format(
        "{}%",
        permille / 10uz);
}

} // namespace ben_bot::pretty_print
