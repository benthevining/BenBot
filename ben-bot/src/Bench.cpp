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

#include <algorithm>
#include <ben-bot/Engine.hpp>
#include <ben-bot/Resources.hpp>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <filesystem>
#include <format>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Result.hpp>
#include <libbenbot/search/Thread.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Files.hpp>
#include <libchess/util/Strings.hpp>
#include <libchess/util/Threading.hpp>
#include <memory>
#include <numeric>
#include <print>
#include <ranges>
#include <string_view>
#include <vector>

namespace ben_bot {

using std::size_t;
using std::string_view;
using uci::printing::info_string;

namespace util     = chess::util;
namespace notation = chess::notation;

namespace {

    // We create a separate context to do bench searches, so that we don't need to
    // mess with changing the main searcher's callbacks, or setting an "isBench"
    // flag to be checked in the callbacks. The simplest approach is to just
    // block in this method - this function creates a search context, executes it,
    // and blocks waiting for the result.

    using SearchResult = search::Result;

    struct BenchSearcherThread final {
        BenchSearcherThread(
            const notation::EPDPosition& position, const size_t defaultDepth)
        {
            thread.set_position(position.position);

            if (const auto it = position.operations.find("depth");
                it != position.operations.end()) {
                thread.context.options.depth = util::int_from_string(it->second, defaultDepth);
            } else {
                thread.context.options.depth = defaultDepth;
            }

            thread.start();
        }

        [[nodiscard]] bool finished() const noexcept { return not thread.context.in_progress(); }

        [[nodiscard]] SearchResult get_result() const noexcept
        {
            assert(finished());

            return result;
        }

    private:
        SearchResult result {};

        search::Thread thread {
            search::Callbacks {
                .onSearchComplete = [this](const SearchResult res) {
                    result = res;
                },
                .onIteration = {} }
        };
    };

    void do_bench(
        const string_view epdText,
        const size_t      defaultDepth)
    {
        const auto searcherThreads = notation::parse_all_epds(epdText)
                                   | std::views::transform([defaultDepth](const notation::EPDPosition& pos) {
                                         return std::make_unique<BenchSearcherThread>(pos, defaultDepth);
                                     })
                                   | std::ranges::to<std::vector>();

        info_string(std::format("Started {} searcher threads", searcherThreads.size()));

        using ThreadPtr = std::unique_ptr<BenchSearcherThread>;

        // wait for all threads to finish searching
        util::progressive_backoff(
            [&searcherThreads] {
                return std::ranges::all_of(searcherThreads,
                    [](const ThreadPtr& thread) { return thread->finished(); });
            });

        const auto results = searcherThreads
                           | std::views::transform([](const ThreadPtr& thread) {
                                 return thread->get_result();
                             })
                           | std::ranges::to<std::vector>();

        const auto totalNodes = std::accumulate(
            results.begin(), results.end(),
            0uz,
            [](const size_t num, const SearchResult& result) {
                return num + result.nodesSearched;
            });

        using std::chrono::milliseconds;

        const auto totalTime = std::accumulate(
            results.begin(), results.end(),
            milliseconds { 0 },
            [](const milliseconds time, const SearchResult& result) {
                return time + result.duration;
            });

        const auto seconds = static_cast<double>(totalTime.count()) * 0.001;

        assert(seconds > 0.);

        const auto nps = static_cast<size_t>(std::round(
            static_cast<double>(totalNodes) / seconds));

        info_string(std::format("Total nodes: {}", totalNodes));
        info_string(std::format("NPS: {}", nps));

        std::println(
            R"-(<DartMeasurement name="Nodes per second" type="numeric/integer">{}</DartMeasurement>)-",
            nps);
    }

} // namespace

void Engine::run_bench(const string_view arguments)
{
    const auto [depth, filePath] = util::split_at_first_space(arguments);

    const auto defaultDepth = util::int_from_string(depth, 3uz);

    if (filePath.empty()) {
        info_string("Running bench for default position set...");
        do_bench(resources::get_bench_epd_text(), defaultDepth);
    } else {
        info_string(std::format("Running bench for {}", filePath));
        do_bench(
            util::load_file_as_string(std::filesystem::path { filePath }),
            defaultDepth);
    }
}

} // namespace ben_bot
