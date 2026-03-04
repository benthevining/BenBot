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

#include <beman/inplace_vector/inplace_vector.hpp>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <format>
#include <iterator>
#include <libbenbot/engine/Engine.hpp>
#include <libchess/uci/Printing.hpp>
#include <libchess/util/Console.hpp>
#include <span>
#include <string>
#include <string_view>

using std::string_view;

namespace {
[[nodiscard]] auto concat_strings(
    const std::span<const string_view> strings)
    -> std::string
{
    std::string result;

    for (const auto fragment : strings) {
        result += fragment;
        result += ' ';
    }

    return result;
}
} // namespace

struct [[nodiscard]] Arguments final {
    /** If true, the executable should process the given UCI command and exit
        immediately, not entering the UCI loop waiting for input.
     */
    bool noLoop { false };

    /** If not empty, this is a one-shot UCI command that should be evaluated
        after startup.
     */
    std::string uciCommand;

    /** Parses the given command-line arguments into a populated Arguments struct. */
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

        // returns true if token is present in argument list, and consumes it if so
        auto check_for_arg = [&args](const string_view token) {
            if (args.empty())
                return false;

            if (args.front() == token) {
                args = args.subspan(1uz);
                return true;
            }

            if (args.back() == token) {
                args = args.first(args.size() - 1uz);
                return true;
            }

            return false;
        };

        return Arguments {
            .noLoop     = check_for_arg("--no-loop"),
            .uciCommand = concat_strings(args)
        };
    }
};

using chess::uci::printing::info_string;

int main(const int argc, const char** argv)
try {
    chess::util::enable_utf8_console_output();

    const auto [noLoop, uciCommand] = Arguments::parse(argc, argv);

    ben_bot::Engine engine;

    if (not uciCommand.empty())
        engine.handle_command(uciCommand);

    if (not noLoop) {
        [[likely]];
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
