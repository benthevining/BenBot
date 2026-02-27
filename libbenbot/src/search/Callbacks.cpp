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

#include <cassert>
#include <chrono>
#include <cmath>
#include <format>
#include <functional>
#include <iostream>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/uci/Printing.hpp>
#include <optional>
#include <print>
#include <ratio>
#include <string>
#include <string_view>
#include <utility>

namespace ben_bot::search {

auto Callbacks::make_uci_printer(
    std::function<bool()>&& isDebugMode)
    -> Callbacks
{
    auto printInfo = [isDebug = std::move(isDebugMode)](const Result& res) {
        search_info(res.to_libchess(isDebug()));
    };

    return {
        .onSearchComplete = [printInfo](const Result& res) {
            printInfo(res);
            chess::uci::printing::best_move(res.best_move(), res.ponder_move()); },
        .onIteration      = printInfo
    };
}

namespace {
    // TODO: do column padding via std::format width specifiers?

    using std::string;

    enum class Alignment {
        Left,
        Right
    };

    template <Alignment Align>
    [[nodiscard]] auto get_column_text(
        const std::string_view text,
        const size_t           totalColumnWidth) -> string
    {
        assert(text.size() < totalColumnWidth);

        const auto trimmedInput = text.substr(0uz, totalColumnWidth);

        string padded;

        if constexpr (Align == Alignment::Left) {
            padded = trimmedInput;

            padded.resize(totalColumnWidth, ' ');
        } else {
            padded.resize(totalColumnWidth - trimmedInput.size(), ' ');

            padded.append(trimmedInput);
        }

        return padded;
    }

    template <Alignment Align>
    void print_column_text(
        const std::string_view text,
        const size_t           totalColumnWidth)
    {
        std::print(
            std::cout,
            "{}",
            get_column_text<Align>(text, totalColumnWidth));
    }

    template <typename Duration>
    [[nodiscard]] auto get_duration_string(
        const milliseconds duration) -> std::optional<string>
    {
        static constexpr auto msPerUnit = duration_cast<milliseconds>(Duration { 1uz });

        if (duration >= msPerUnit) {
            using FractionalDuration = std::chrono::duration<float, typename Duration::period>;

            return std::format(
                "{:.2%Q %q}",
                duration_cast<FractionalDuration>(duration));
        }

        return std::nullopt;
    }

    [[nodiscard]] auto format_duration(
        const milliseconds duration) -> string
    {
        // NB. it should be quite rare that a search will run for 1 day or more...
        return get_duration_string<std::chrono::hours>(duration)
            .or_else([duration] { return get_duration_string<std::chrono::minutes>(duration); })
            .or_else([duration] { return get_duration_string<std::chrono::seconds>(duration); })
            .or_else([duration] { return std::make_optional(std::format("{:%Q %q}", duration)); })
            .value();
    }

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

    template <size_t Precision = 2uz>
    [[nodiscard]] auto format_nodes(
        const size_t nodes) -> string
    {
        return get_nodes_string<std::mega, 'M', Precision>(nodes)
            .or_else([nodes] { return get_nodes_string<std::kilo, 'k', Precision>(nodes); })
            .or_else([nodes] { return std::make_optional(std::format("{}", nodes)); })
            .value();
    }

    [[nodiscard, gnu::const]] auto get_nps(const Result& res) -> size_t
    {
        const auto seconds = static_cast<double>(res.duration.count()) * 0.001;

        if (seconds <= 0.)
            return 0uz;

        const auto nps = static_cast<double>(res.nodesSearched) / seconds;

        return static_cast<size_t>(std::round(nps));
    }

    [[nodiscard]] auto format_nps(const size_t nps) -> string
    {
        return std::format(
            "{}/s",
            format_nodes<1uz>(nps));
    }

    constexpr auto COL_DEPTH = 10uz;
    constexpr auto COL_TIME  = 10uz;
    constexpr auto COL_NODES = 10uz;
    constexpr auto COL_NPS   = 10uz;

    void pretty_print(const Result& res)
    {
        // depth
        print_column_text<Alignment::Left>(
            std::format("{}/{}", res.depth, res.qDepth),
            COL_DEPTH);

        // time
        print_column_text<Alignment::Right>(
            format_duration(res.duration),
            COL_TIME);

        // nodes
        print_column_text<Alignment::Right>(
            format_nodes(res.nodesSearched),
            COL_NODES);

        // nodes per second
        print_column_text<Alignment::Right>(
            format_nps(get_nps(res)),
            COL_NPS);

        // final newline
        std::print(std::cout, "\n");
    }
} // namespace

auto Callbacks::make_pretty_printer()
    -> Callbacks
{
    return {
        .onSearchComplete = pretty_print,
        .onIteration      = pretty_print
    };
}

} // namespace ben_bot::search
