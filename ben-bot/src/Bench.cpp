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

#include "Data.hpp"
#include "Engine.hpp"
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <libbenbot/search/Search.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/util/Files.hpp>
#include <libchess/util/Strings.hpp>
#include <print>
#include <string_view>

namespace ben_bot {

using std::println;
using std::size_t;
using std::string_view;

namespace util = chess::util;

namespace {

    // We create a separate context to do bench searches, so that we don't need to
    // mess with changing the main searcher's callbacks, or setting an "isBench"
    // flag to be checked in the callbacks. The simplest approach is to just
    // block in this method - this function creates a search context, executes it,
    // and blocks waiting for the result.

    void do_bench(
        const string_view epdText,
        const size_t      defaultDepth)
    {
        auto totalNodes { 0uz };

        std::chrono::milliseconds totalTime { 0 };

        search::Context benchSearcher {
            search::Callbacks {
                .onSearchComplete = [&totalNodes, &totalTime](const search::Callbacks::Result& res) {
                    totalNodes += res.nodesSearched;
                    totalTime += res.duration;
                } }
        };

        auto posNum { 0uz };

        for (const auto& position : chess::notation::parse_all_epds(epdText)) {
            benchSearcher.options.position = position.position;
            benchSearcher.options.movesToSearch.clear();

            if (const auto it = position.operations.find("depth");
                it != position.operations.end()) {
                benchSearcher.options.depth = util::int_from_string(it->second, defaultDepth);
            } else {
                benchSearcher.options.depth = defaultDepth;
            }

            println("Searching for position #{}...", posNum);

            benchSearcher.search();

            ++posNum;
        }

        const auto seconds = static_cast<double>(totalTime.count()) * 0.001;

        assert(seconds > 0.);

        const auto nps = static_cast<size_t>(std::round(
            static_cast<double>(totalNodes) / seconds));

        println("Total nodes: {}", totalNodes);
        println("NPS: {}", nps);

        println(
            R"-(<DartMeasurement name="Nodes per second" type="numeric/integer">{}</DartMeasurement>)-",
            nps);
    }

} // namespace

void Engine::run_bench(const string_view arguments)
{
    const auto [depth, filePath] = util::split_at_first_space(arguments);

    const auto defaultDepth = util::int_from_string(depth, 3uz);

    if (filePath.empty()) {
        do_bench(get_bench_epd_text(), defaultDepth);
    } else {
        do_bench(
            util::load_file_as_string(path { filePath }),
            defaultDepth);
    }
}

} // namespace ben_bot
