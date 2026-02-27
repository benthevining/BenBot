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
#include <libchess/moves/Move.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Variant.hpp>
#include <optional>
#include <print>
#include <ratio>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace ben_bot::search {

auto Callbacks::make_uci_printer(
    std::function<bool()>&& isDebugMode)
    -> Callbacks
{
    auto printInfo = [isDebug = std::move(isDebugMode)](const Result& res) {
        search_info(res.to_libchess(isDebug()));
    };

    return {
        .onSearchStart    = nullptr,
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
        Right,
        Center
    };

    constexpr auto COLUMN_WIDTH = 10uz;

    template <Alignment Align>
    [[nodiscard]] auto get_column_text(
        const std::string_view text) -> string
    {
        assert(text.size() < COLUMN_WIDTH);

        const auto trimmedInput = text.substr(0uz, COLUMN_WIDTH);

        string padded;

        if constexpr (Align == Alignment::Left) {
            padded = trimmedInput;

            padded.resize(COLUMN_WIDTH, ' ');
        } else {
            auto padding = COLUMN_WIDTH - trimmedInput.size();

            if constexpr (Align == Alignment::Center) {
                padding /= 2uz;
            }

            padded.resize(padding, ' ');

            padded.append(trimmedInput);

            if constexpr (Align == Alignment::Center) {
                padded.append(COLUMN_WIDTH - padded.size(), ' ');
            }
        }

        return padded;
    }

    template <Alignment Align>
    void print_column_text(
        const std::string_view text)
    {
        std::print(
            std::cout,
            "{}",
            get_column_text<Align>(text));
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
            format_nodes<1uz>(nps)); // use this function for its transformation of the value to a k/M representation
    }

    [[nodiscard]] auto format_hashfull(const size_t permille) -> string
    {
        return std::format(
            "{}%",
            permille / 10uz);
    }

    [[nodiscard, gnu::const]] auto plies_to_moves(int plies) noexcept -> int
    {
        if (std::cmp_greater(plies, 0))
            ++plies;

        return plies / 2;
    }

    using Score = chess::uci::printing::SearchInfo::Score;

    [[nodiscard]] auto format_score(
        const Score& score) -> string
    {
        return std::visit(
            chess::util::Visitor {
                [](const Score::Centipawns& centipawns) {
                    return std::format(
                        "{:+}",
                        centipawns.value);
                },
                [](const Score::MateIn& mate) {
                    return std::format(
                        "#{}",
                        std::abs(plies_to_moves(mate.plies)));
                } },
            score.value);
    }

    [[nodiscard]] auto format_pv(
        const std::span<const Move> pv) -> string
    {
        if (pv.empty()) {
            // this is possible if we're checkmated
            return { };
        }

        string result;

        for (const auto move : pv) {
            result.append(chess::notation::to_uci(move));
            result.append(1uz, ' ');
        }

        result.pop_back(); // trim last space

        return result;
    }

    void print_table_header()
    {
        print_column_text<Alignment::Center>("Depth");

        print_column_text<Alignment::Right>("Time");

        print_column_text<Alignment::Right>("Nodes");

        print_column_text<Alignment::Center>("NPS");

        print_column_text<Alignment::Center>("Hashfull");

        print_column_text<Alignment::Center>("Score");

        std::println(
            std::cout,
            "{}",
            "PV");
    }

    void pretty_print(const Result& res)
    {
        // depth
        print_column_text<Alignment::Center>(
            std::format("{}/{}", res.depth, res.qDepth));

        // time
        print_column_text<Alignment::Right>(
            format_duration(res.duration));

        // nodes
        print_column_text<Alignment::Right>(
            format_nodes(res.nodesSearched));

        // nodes per second
        print_column_text<Alignment::Right>(
            format_nps(get_nps(res)));

        // hashfull
        print_column_text<Alignment::Center>(
            format_hashfull(res.hashfull));

        // score
        print_column_text<Alignment::Center>(
            format_score(res.score.to_libchess()));

        // PV
        std::println(
            std::cout,
            "{}",
            format_pv(res.pv));
    }
} // namespace

auto Callbacks::make_pretty_printer()
    -> Callbacks
{
    return {
        .onSearchStart = []([[maybe_unused]] const Options& options) {
            print_table_header();
        },
        .onSearchComplete = pretty_print,
        .onIteration      = pretty_print
    };
}

} // namespace ben_bot::search
