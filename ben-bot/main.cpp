/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <atomic>
#include <chrono>
#include <cstddef> // IWYU pragma: keep - for size_t
#include <cstdlib>
#include <exception>
#include <future>
#include <libchess/game/Position.hpp>
#include <libchess/game/TimeControl.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/search/Thread.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <optional>
#include <print>
#include <string_view>
#include <utility>

namespace chess {

using game::Position;
using pieces::Color;
using std::size_t;

using Milliseconds = std::chrono::milliseconds;

class BenBotEngine final : public uci::EngineBase {
    [[nodiscard]] std::string_view get_name() const override { return "BenBot"; }

    [[nodiscard]] std::string_view get_author() const override { return "Ben Vining"; }

    void new_game() override { searcherThread.new_game(); }

    void set_position(const Position& pos) override { position = pos; }

    void go(const uci::GoCommandOptions& opts) override
    {
        const auto maxDepth = opts.depth.or_else([] {
                                            return std::optional { 4uz };
                                        })
                                  .value();

        const auto searchTime = opts.searchTime.or_else(
            [&opts, isWhite = position.sideToMove == Color::White] {
                const auto& timeLeft = isWhite ? opts.whiteTimeLeft : opts.blackTimeLeft;

                if (! timeLeft.has_value())
                    return std::optional<Milliseconds> {};

                const auto& inc = isWhite ? opts.whiteInc : opts.blackInc;

                const auto incValue = inc.or_else([] {
                                             return std::optional { Milliseconds { 0 } };
                                         })
                                          .value();

                return std::optional {
                    game::determine_search_time(*timeLeft, incValue)
                };
            });

        std::promise<moves::Move> result;

        auto future = result.get_future();

        searcherThread.run(
            std::move(result), position, maxDepth, searchTime);

        future.wait();

        std::println("bestmove {}",
            notation::to_uci(future.get()));
    }

    void abort_search() override { searcherThread.interrupt(); }
    void stop_search() override { searcherThread.interrupt(); }

    void wait() override { searcherThread.wait(); }

    search::Thread searcherThread;

    Position position;
};

} // namespace chess

int main(
    [[maybe_unused]] const int argc, [[maybe_unused]] const char** argv)
try {
    chess::BenBotEngine engine;

    engine.loop();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
