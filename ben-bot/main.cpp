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
#include <beman/inplace_vector/inplace_vector.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint> // IWYU pragma: keep - for std::uint_least8_t
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iterator>
#include <libbenbot/engine/Engine.hpp>
#include <libchess/uci/Printing.hpp>
#include <libutil/Console.hpp>
#include <libutil/Environment.hpp>
#include <span>
#include <string>
#include <string_view>

namespace {
using std::string_view;

struct [[nodiscard]] Arguments final {
    enum class RunMode : std::uint_least8_t {
        Bench,
        UCILoop
    };

    RunMode runMode { RunMode::UCILoop };

    [[nodiscard]] static auto parse(
        const int argc, const char** argv) -> Arguments
    {
        assert(argc > 0);
        assert(argv != nullptr);

        static constexpr auto MAX_ARGS = 128uz;

        const beman::inplace_vector::inplace_vector<string_view, MAX_ARGS> argStorage {
            argv,
            std::next(argv, static_cast<std::ptrdiff_t>(argc))
        };

        std::span args { argStorage };

        // consume program name
        args = args.subspan(1uz);

        Arguments result;

        if (std::ranges::contains(args, "bench"))
            result.runMode = RunMode::Bench;

        return result;
    }
};
} // namespace

using chess::uci::printing::info_string;

int main(const int argc, const char** argv)
try {
    util::enable_utf8_console_output();

    const auto [mode] = Arguments::parse(argc, argv);

    ben_bot::Engine engine;

    util::get_environment_variable("BENBOT_CONFIG")
        .transform([&engine](const string_view value) {
            if (not value.empty())
                engine.read_config_file(std::filesystem::path { value });

            return std::monostate { };
        });

    switch (mode) {
        using enum Arguments::RunMode;

        case Bench:
            engine.handle_command("bench");
            break;

        default: [[fallthrough]];
        case UCILoop:
            engine.loop();
    }

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    info_string(std::format("Internal error: {}", exception.what()));
    return EXIT_FAILURE;
} catch (...) {
    info_string("Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
