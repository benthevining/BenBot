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
#include <cassert>
#include <chrono>
#include <cmath>   // IWYU pragma: keep - for std::round()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <expected>
#include <filesystem>
#include <format>
#include <functional>
#include <libbenbot/Resources.hpp>
#include <libbenbot/engine/Engine.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Options.hpp>
#include <libbenbot/search/Result.hpp>
#include <libbenbot/search/Thread.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Chrono.hpp>
#include <libutil/Files.hpp>
#include <libutil/Strings.hpp>
#include <libutil/Threading.hpp>
#include <memory>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ben_bot {

using std::size_t;
using std::string_view;
using uci::printing::info_string;

namespace notation = chess::notation;

namespace {

    // We create a separate context to do bench searches, so that we don't need to
    // mess with changing the main searcher's callbacks, or setting an "isBench"
    // flag to be checked in the callbacks.

    using SearchResult = search::Result;

    struct [[nodiscard]] BenchSearcherThread final {
        BenchSearcherThread(
            const size_t                 threadNum,
            const notation::EPDPosition& position,
            const size_t                 defaultDepth,
            const bool                   printProgressOutput)
            : threadNumber { threadNum }
            , outputProgress { printProgressOutput }
        {
            thread.context.set_position(position.position);

            search::Options options;

            if (const auto it = position.operations.find("depth");
                it != position.operations.end()) {
                options.depth = util::strings::int_from_string(it->second, defaultDepth);
            } else {
                options.depth = defaultDepth;
            }

            thread.context.set_options(options);

            thread.start();
        }

        BenchSearcherThread(const BenchSearcherThread&)            = delete;
        BenchSearcherThread& operator=(const BenchSearcherThread&) = delete;

        BenchSearcherThread(BenchSearcherThread&&)            = delete;
        BenchSearcherThread& operator=(BenchSearcherThread&&) = delete;

        ~BenchSearcherThread() = default;

        [[nodiscard]] auto finished() const noexcept -> bool { return not thread.context.in_progress(); }

        [[nodiscard]] auto get_result() const noexcept -> SearchResult
        {
            assert(finished());

            return result;
        }

    private:
        size_t threadNumber { 0uz };

        bool outputProgress { false };

        SearchResult result;

        // clang-format off
        search::Thread thread {
            search::Callbacks {
                .onSearchStart    = nullptr,
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

            search_info(info);
        }
    };

    [[nodiscard]] auto get_bench_results(
        const string_view epdText,
        const size_t      defaultDepth,
        const bool        printProgressOutput) -> std::vector<SearchResult>
    {
        const auto epds = notation::parse_all_epds(epdText);

        using ThreadPtr = std::unique_ptr<BenchSearcherThread>;

        std::vector<ThreadPtr> searcherThreads;

        searcherThreads.reserve(epds.size());

        for (auto idx = 0uz; idx < epds.size(); ++idx) {
            searcherThreads.emplace_back(
                std::make_unique<BenchSearcherThread>(
                    idx + 1uz, // display 1-based thread numbers
                    epds.at(idx), defaultDepth, printProgressOutput));
        }

        // wait for all threads to finish searching
        util::progressive_backoff(
            [&searcherThreads] {
                return std::ranges::all_of(searcherThreads,
                    [](const ThreadPtr& thread) { return thread->finished(); });
            });

        return searcherThreads
             | std::views::transform([](const ThreadPtr& thread) {
                   return thread->get_result();
               })
             | std::ranges::to<std::vector>();
    }

    void do_bench(
        const string_view epdText,
        const size_t      defaultDepth,
        const bool        printProgressOutput)
    {
        using std::chrono::milliseconds;

        const auto results = get_bench_results(epdText, defaultDepth, printProgressOutput);

        const auto totalNodes = std::transform_reduce(
            results.begin(), results.end(),
            0uz,
            std::plus<void> { },
            [](const SearchResult& result) { return result.nodesSearched; });

        const auto totalTime = std::transform_reduce(
            results.begin(), results.end(),
            milliseconds { 0 },
            std::plus<void> { },
            [](const SearchResult& result) { return result.duration; });

        using Seconds = util::FractionalDuration<std::chrono::seconds>;

        const auto seconds = duration_cast<Seconds>(totalTime);

        assert(seconds > Seconds { 0.f });

        const auto nps = static_cast<size_t>(std::round(
            static_cast<Seconds::rep>(totalNodes) / seconds.count()));

        info_string(std::format("Total nodes: {}", totalNodes));
        info_string(std::format("Total seconds: {}", seconds));
        info_string(std::format("NPS: {}", nps));
    }

} // namespace

void Engine::run_bench(const string_view arguments) const
{
    const auto [depth, filePath] = util::strings::split_at_first_space(arguments);

    const auto defaultDepth = util::strings::int_from_string(depth, 3uz);

    if (filePath.empty()) {
        info_string("Running bench for default position set...");

        do_bench(
            resources::get_bench_epd_text(),
            defaultDepth, is_debug_mode());

        return;
    }

    const auto epdPath = absolute(std::filesystem::path { filePath });

    [[maybe_unused]] const auto result
        = util::files::load(epdPath)
              .transform([this, defaultDepth, absPathStr = epdPath.string()](const string_view fileContent) {
                  info_string(std::format("Running bench for {}...", absPathStr));

                  do_bench(fileContent, defaultDepth, is_debug_mode());
              })
              .transform_error(info_string);
}

} // namespace ben_bot
