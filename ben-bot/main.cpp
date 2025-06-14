/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <cstdlib>
#include <exception>
#include <libchess/game/Position.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <print>
#include <string_view>

namespace chess {

class BenBotEngine final : public uci::EngineBase {
    [[nodiscard]] std::string_view get_name() const override { return "BenBot"; }

    [[nodiscard]] std::string_view get_author() const override { return "Ben Vining"; }

    void new_game() override { searchContext.transTable.clear(); }

    void set_position(const game::Position& pos) override { searchContext.options.position = pos; }

    void go(const uci::GoCommandOptions& opts) override
    {
        opts.update_search_options(
            searchContext.options,
            searchContext.options.position.sideToMove == pieces::Color::White);

        searchContext.search();
    }

    search::Context<true> searchContext;
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
