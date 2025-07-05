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

#include <atomic>
#include <cstdlib>
#include <exception>
#include <libbenbot/search/Search.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <print>
#include <string_view>
#include <thread>
#include <utility>

namespace chess {

class BenBotEngine final : public uci::EngineBase {
public:
    BenBotEngine()
    {
        searchContext.callbacks = search::Callbacks::make_uci_handler();
    }

    ~BenBotEngine() override
    {
        threadShouldExit.store(true);
        searchContext.abort();
        searcherThread.join();
    }

    BenBotEngine(const BenBotEngine&)            = delete;
    BenBotEngine& operator=(const BenBotEngine&) = delete;
    BenBotEngine(BenBotEngine&&)                 = delete;
    BenBotEngine& operator=(BenBotEngine&&)      = delete;

private:
    [[nodiscard]] std::string_view get_name() const override { return "BenBot"; }

    [[nodiscard]] std::string_view get_author() const override { return "Ben Vining"; }

    void new_game() override { searchContext.reset(); }

    void set_position(const game::Position& pos) override
    {
        searchContext.wait();
        searchContext.options.position = pos;
    }

    void go(uci::GoCommandOptions&& opts) override
    {
        searchContext.wait();

        searchContext.options.update_from(std::move(opts));

        startSearch.store(true);
    }

    void abort_search() override { searchContext.abort(); }

    void wait() override { searchContext.wait(); }

    void thread_func()
    {
        while (! threadShouldExit.load()) {
            if (startSearch.exchange(false))
                searchContext.search();
            else
                std::this_thread::yield();
        }
    }

    search::Context searchContext;

    std::thread searcherThread { [this] { thread_func(); } };

    std::atomic_bool threadShouldExit { false };
    std::atomic_bool startSearch { false };
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
