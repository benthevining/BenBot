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
#include <libchess/notation/UCI.hpp>
#include <libchess/search/Search.hpp>
#include <libchess/search/TranspositionTable.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <print>
#include <string_view>

class BenBotEngine final : public chess::uci::EngineBase {
    using Position = chess::game::Position;

    [[nodiscard]] std::string_view get_name() const override { return "BenBot"; }

    [[nodiscard]] std::string_view get_author() const override { return "Ben Vining"; }

    void new_game() override { transTable.clear(); }

    void set_position(const Position& pos) override { position = pos; }

    void go([[maybe_unused]] const chess::uci::GoCommandOptions& opts) override
    {
        std::println("bestmove {}",
            chess::notation::to_uci(
                chess::search::find_best_move(position, transTable)));
    }

    Position position;

    chess::search::TranspositionTable transTable;
};

int main(
    [[maybe_unused]] const int argc, [[maybe_unused]] const char** argv)
try {
    BenBotEngine engine;

    engine.loop();

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println("{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
