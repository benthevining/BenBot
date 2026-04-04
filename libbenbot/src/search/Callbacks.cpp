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
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <format>
#include <functional>
#include <iostream>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/PrettyPrinting.hpp>
#include <libbenbot/search/Result.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/uci/Printing.hpp>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <termcolor/termcolor.hpp>
#include <utility>

namespace ben_bot::search {

auto Callbacks::make_uci_printer(
    std::function<bool()> isDebugMode)
    -> Callbacks
{
    namespace uci_printing = chess::uci::printing;

    auto printInfo = [isDebugMode](const Result& res) {
        search_info(res.to_libchess(isDebugMode()));
    };

    return {
        .onSearchStart    = nullptr,
        .onSearchComplete = [printInfo](const Result& res) {
            printInfo(res);
            uci_printing::best_move(res.best_move(), res.ponder_move()); },
        .onIteration      = printInfo,
        .onRootMove       = [isDebugMode](const Move move, const size_t idx) {
            if (isDebugMode()) {
                uci_printing::currmove_info(
                    move,
                    idx + 1uz); // convert 0-based -> 1-based index
            } }
    };
}

namespace {
    using std::string;
    using std::string_view;

    enum class Alignment : std::uint_least8_t {
        Left,
        Right,
        Center
    };

    inline constexpr auto COLUMN_WIDTH = 10uz;

    template <Alignment Align>
    void print_column_text(
        const string_view text)
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

        std::cout << std::format(
            formatStr,
            text.substr(0uz, COLUMN_WIDTH),
            COLUMN_WIDTH);
    }

    using Score = chess::uci::printing::SearchInfo::Score;

    void print_score(
        const Score& score)
    {
        switch (score.get_type()) {
            using enum Score::Type;

            case Winning:
                std::cout << termcolor::green;
                break;
            case Losing:
                std::cout << termcolor::red;
                break;
            default: [[fallthrough]];
            case Equal:
                std::cout << termcolor::grey;
                break;
        }

        print_column_text<Alignment::Center>(
            pretty_print::evaluation(score));

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

        for (const auto move : pv)
            result.append(std::format("{} ", printMove(move)));

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

        print_column_text<Alignment::Center>("TT hits");

        print_column_text<Alignment::Center>("Beta cutoffs");

        print_column_text<Alignment::Center>("MDP cutoffs");

        print_column_text<Alignment::Center>("Static evals");

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
            pretty_print::duration(res.duration));

        // nodes
        print_column_text<Alignment::Right>(
            pretty_print::nodes(res.nodesSearched));

        const auto libchess = res.to_libchess(false);

        // nodes per second
        print_column_text<Alignment::Right>(
            pretty_print::nps(libchess.get_nps()));

        // hashfull
        print_column_text<Alignment::Center>(
            pretty_print::hashfull(res.hashfull));

        // TT hits
        print_column_text<Alignment::Center>(
            pretty_print::search_stat(
                res.transpositionTableHits, res.nodesSearched));

        // beta cutoffs
        print_column_text<Alignment::Center>(
            pretty_print::search_stat(
                res.betaCutoffs, res.nodesSearched));

        // MDP cutoffs
        print_column_text<Alignment::Center>(
            pretty_print::search_stat(
                res.mdpCutoffs, res.nodesSearched));

        // static evals
        print_column_text<Alignment::Center>(
            pretty_print::search_stat(
                res.staticEvals, res.nodesSearched));

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
        .onIteration      = printIteration,
        .onRootMove       = nullptr
    };
}

} // namespace ben_bot::search
