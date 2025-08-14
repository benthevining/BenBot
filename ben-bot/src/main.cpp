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

#include "Engine.hpp"
#include <beman/inplace_vector/inplace_vector.hpp>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iterator>
#include <libchess/util/Console.hpp>
#include <print>
#include <span>
#include <string>
#include <string_view>

using std::string_view;

namespace {
[[nodiscard]] std::string concat_strings(
    const std::span<const string_view> strings)
{
    std::string result;

    for (const auto fragment : strings) {
        result += fragment;
        result += ' ';
    }

    return result;
}
} // namespace

int main(const int argc, const char** argv)
try {
    chess::util::enable_utf8_console_output();

    // handle command line args
    // we process any args as a one-shot UCI command line
    // --no-loop can also be given to make the engine exit after processing the given CLI command
    // --no-logo will suppress the logo & version normally printed at startup

    static constexpr auto MAX_ARGS = 128uz;

    const beman::inplace_vector<string_view, MAX_ARGS> argStorage {
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

    const auto noLoop = check_for_arg("--no-loop");
    const auto noLogo = check_for_arg("--no-logo");

    ben_bot::Engine engine;

    if (not noLogo) {
        [[likely]];
        engine.print_logo_and_version();
    }

    if (not args.empty())
        engine.handle_command(concat_strings(args));

    if (not noLoop) {
        [[likely]];
        engine.loop();
    }

    return EXIT_SUCCESS;
} catch (const std::exception& exception) {
    std::println(std::cerr, "{}", exception.what());
    return EXIT_FAILURE;
} catch (...) {
    std::println(std::cerr, "Error: unknown exception thrown!");
    return EXIT_FAILURE;
}
