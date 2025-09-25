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
    // flag to be checked in the callbacks.

    using SearchResult = search::Result;

    struct BenchSearcherThread final {
        BenchSearcherThread(
            const size_t                 threadNum,
            const notation::EPDPosition& position,
            const size_t                 defaultDepth,
            const bool                   printProgressOutput)
            : threadNumber { threadNum }
            , outputProgress { printProgressOutput }
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
        size_t threadNumber { 0uz };

        bool outputProgress { false };

        SearchResult result {};

        // clang-format off
        search::Thread thread {
            search::Callbacks {
                .onSearchComplete = [this](const SearchResult& res) { print_info(res); result = res; },
                .onIteration      = [this](const SearchResult& res) { print_info(res); }
            }
        };
        // clang-format on

        void print_info(const SearchResult& res) const
        {
            if (not outputProgress)
                return;

            auto info = res.to_libchess(false);

            info.extraInformation = std::format("thread {}", threadNumber);

            uci::printing::search_info(info);
        }
    };

    void do_bench(
        const string_view epdText,
        const size_t      defaultDepth,
        const bool        printProgressOutput)
    {
        using ThreadPtr = std::unique_ptr<BenchSearcherThread>;

        std::vector<ThreadPtr> searcherThreads;

        {
            const auto epds = notation::parse_all_epds(epdText);

            info_string(std::format("Starting {} searcher threads", epds.size()));

            for (auto i = 0uz; i < epds.size(); ++i) {
                searcherThreads.emplace_back(
                    std::make_unique<BenchSearcherThread>(
                        i + 1uz, // display 1-based thread numbers
                        epds.at(i), defaultDepth, printProgressOutput));
            }
        }

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

        // CTest can parse test output to extract custom test measurements, which CDash can track over time
        // see https://cmake.org/cmake/help/latest/command/ctest_test.html#additional-test-measurements
        info_string(std::format(
            R"-(<DartMeasurement name="Nodes per second" type="numeric/integer">{}</DartMeasurement>)-",
            nps));
    }

} // namespace

void Engine::run_bench(const string_view arguments) const
{
    const auto [depth, filePath] = util::split_at_first_space(arguments);

    const auto defaultDepth = util::int_from_string(depth, 3uz);

    if (filePath.empty()) {
        info_string("Running bench for default position set...");

        do_bench(
            resources::get_bench_epd_text(),
            defaultDepth, debugMode.load());

        return;
    }

    const auto epdPath = absolute(std::filesystem::path { filePath });

    const auto absPathStr = epdPath.string(); // NOLINT(build/include_what_you_use)

    info_string(std::format("Running bench for {}...", absPathStr));

    // output the filename for CTest to detect & upload with the test info
    // see https://cmake.org/cmake/help/latest/command/ctest_test.html#attached-files
    info_string(std::format(
        R"-(<CTestMeasurementFile type="file" name="BenchData">{}</CTestMeasurementFile>)-",
        absPathStr));

    do_bench(
        util::load_file_as_string(epdPath),
        defaultDepth, debugMode.load());
}

} // namespace ben_bot
