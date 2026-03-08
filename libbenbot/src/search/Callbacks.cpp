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
#include <cmath>   // IWYU pragma: keep - for std::abs()
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <format>
#include <functional>
#include <iostream>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Chrono.hpp>
#include <libchess/util/Variant.hpp>
#include <optional>
#include <ratio>
#include <span>
#include <string>
#include <string_view>
#include <termcolor/termcolor.hpp>
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
    using std::string;

    enum class Alignment : std::uint_least8_t {
        Left,
        Right,
        Center
    };

    inline constexpr auto COLUMN_WIDTH = 10uz;

    template <Alignment Align>
    [[nodiscard]] auto get_column_text(
        const std::string_view text) -> string
    {
        assert(text.size() < COLUMN_WIDTH);

        static constexpr auto formatStr = [] {
            if constexpr (Align == Alignment::Left) {
                return "{:<{}}";
            } else if constexpr (Align == Alignment::Center) {
                return "{:^{}}";
            } else {
                static_assert(Align == Alignment::Right);
                return "{:>{}}";
            }
        }();

        return std::format(
            formatStr,
            text.substr(0uz, COLUMN_WIDTH),
            COLUMN_WIDTH);
    }

    template <Alignment Align>
    void print_column_text(
        const std::string_view text)
    {
        std::cout << get_column_text<Align>(text);
    }

    template <chess::util::ChronoDuration Duration>
    [[nodiscard]] auto get_duration_string(
        const milliseconds duration) -> std::optional<string>
    {
        static constexpr auto msPerUnit = duration_cast<milliseconds>(Duration { 1uz });

        if (duration >= msPerUnit) {
            using FractionalDuration = chess::util::FractionalDuration<Duration>;

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
                        std::abs(mate.moves()));
                } },
            score.value);
    }

    void print_score(
        const Score& score)
    {
        enum class ScoreType : std::uint_least8_t {
            Winning,
            Losing,
            Equal
        };

        const auto type = std::visit(
            chess::util::Visitor {
                [](const Score::Centipawns& value) {
                    if (value.value == 0)
                        return ScoreType::Equal;

                    if (value.value > 0)
                        return ScoreType::Winning;

                    return ScoreType::Losing;
                },
                [](const Score::MateIn& mate) {
                    if (mate.plies > 0)
                        return ScoreType::Winning;

                    return ScoreType::Losing;
                } },
            score.value);

        switch (type) {
            case ScoreType::Winning:
                std::cout << termcolor::green;
                break;
            case ScoreType::Losing:
                std::cout << termcolor::red;
                break;
            default: [[fallthrough]];
            case ScoreType::Equal:
                std::cout << termcolor::grey;
                break;
        }

        print_column_text<Alignment::Center>(
            format_score(score));

        std::cout << termcolor::reset;
    }

    using MovePrinter = std::function<std::string(Move)>;

    [[nodiscard]] auto format_pv(
        const std::span<const Move> pv,
        const MovePrinter&          printMove) -> string
    {
        if (pv.empty()) {
            // this is possible if we're checkmated
            return { };
        }

        string result;

        for (const auto move : pv) {
            result.append(printMove(move));
            result.append(1uz, ' ');
        }

        result.pop_back(); // trim last space

        return result;
    }

    void print_table_header()
    {
        std::cout << termcolor::bold;

        print_column_text<Alignment::Center>("Depth");

        print_column_text<Alignment::Right>("Time");

        print_column_text<Alignment::Right>("Nodes");

        print_column_text<Alignment::Center>("NPS");

        print_column_text<Alignment::Center>("Hashfull");

        print_column_text<Alignment::Center>("Score");

        std::cout << "PV\n"
                  << termcolor::reset;
    }

    void pretty_print(
        const Result& res, const MovePrinter& printMove)
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

        const auto libchess = res.to_libchess(false);

        // nodes per second
        print_column_text<Alignment::Right>(
            format_nps(libchess.get_nps()));

        // hashfull
        print_column_text<Alignment::Center>(
            format_hashfull(res.hashfull));

        // score
        print_score(libchess.score);

        // PV
        // NB. passing PV to operator<< by value gives a warning on MSVC
        const auto pv = format_pv(res.pv, printMove);
        std::cout << pv << '\n';
    }
} // namespace

auto Callbacks::make_pretty_printer(
    MovePrinter&& printMove)
    -> Callbacks
{
    auto printIteration = [formatMove = std::move(printMove)](const Result& res) {
        pretty_print(res, formatMove);
    };

    return {
        .onSearchStart = []([[maybe_unused]] const Options& options) {
            print_table_header();
        },
        .onSearchComplete = printIteration,
        .onIteration      = printIteration
    };
}

} // namespace ben_bot::search
