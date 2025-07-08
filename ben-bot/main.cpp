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

#include <array>
#include <cmrc/cmrc.hpp>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <libbenbot/data-structures/OpeningBook.hpp>
#include <libbenbot/search/Search.hpp>
#include <libbenbot/search/Thread.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Magics.hpp>
#include <libchess/uci/CommandParsing.hpp>
#include <libchess/uci/EngineBase.hpp>
#include <libchess/uci/Options.hpp>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <utility>

CMRC_DECLARE(ben_bot_resources);

namespace {
[[nodiscard]] std::string load_file_as_string(const std::filesystem::path& file)
{
    std::ifstream input { file };

    input.exceptions(
        std::ios_base::badbit | std::ios_base::failbit);

    using Iterator = std::istreambuf_iterator<char>;

    return { Iterator { input }, Iterator {} };
}
} // namespace

namespace chess {

using std::string_view;

class BenBotEngine final : public uci::EngineBase {
    [[nodiscard]] string_view get_name() const override { return "BenBot"; }

    [[nodiscard]] string_view get_author() const override { return "Ben Vining"; }

    void new_game() override
    {
        moves::magics::init();

        searcher.context.reset(); // clears transposition table

        if (! bookLoaded) { // load embedded book data into opening book data structure
            const auto bookFile = cmrc::ben_bot_resources::get_filesystem()
                                      .open("book.json");

            searcher.context.openingBook.book.add_from_json(string_view { bookFile });

            bookLoaded = true;
        }
    }

    // this function implements non-standard UCI commands that we support
    void handle_custom_command(const string_view command, const string_view options) override
    {
        if (command == "loadbook")
            load_book_file(std::filesystem::path { options });
    }

    void set_position(const game::Position& pos) override { searcher.set_position(pos); }

    void go(uci::GoCommandOptions&& opts) override { searcher.start(std::move(opts)); }

    void abort_search() override { searcher.context.abort(); }

    void wait() override { searcher.context.wait(); }

    [[nodiscard]] std::span<uci::Option*> get_options() override { return options; }

    void load_book_file(const std::filesystem::path& file)
    try {
        wait();

        searcher.context.openingBook.book.add_from_json(
            load_file_as_string(file));
    } catch (const std::exception& except) {
        std::println(
            "Error reading from opening book file at path: {}",
            file.string());

        std::println("{}", except.what());
    }

    search::Thread searcher { search::Callbacks::make_uci_handler() };

    bool bookLoaded { false };

    uci::Action clearTT {
        "Clear Hash",
        [this] { searcher.context.reset(); }
    };

    std::array<uci::Option*, 2uz> options {
        &searcher.context.openingBook.enabled,
        &clearTT
    };
};

} // namespace chess

int main(
    [[maybe_unused]] const int    argc,
    [[maybe_unused]] const char** argv)
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
