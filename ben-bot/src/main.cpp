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

    ben_bot::Engine engine;

    engine.print_logo_and_version();

    // handle command line args
    // we process any args as a one-shot UCI command line
    // --no-loop can also be given to make the engine exit after processing the given CLI command

    static constexpr auto MAX_ARGS = 128uz;

    const beman::inplace_vector<string_view, MAX_ARGS> argStorage {
        argv,
        std::next(argv, static_cast<std::ptrdiff_t>(argc))
    };

    std::span args { argStorage };

    // consume program name
    args = args.subspan(1uz);

    // check for --no-loop token, consume it if found
    const auto noLoop = [&args] {
        static constexpr string_view NO_LOOP_TOKEN { "--no-loop" };

        if (args.front() == NO_LOOP_TOKEN) {
            args = args.subspan(1uz);
            return true;
        }

        if (args.back() == NO_LOOP_TOKEN) {
            args = args.first(args.size() - 1uz);
            return true;
        }

        return false;
    }();

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
