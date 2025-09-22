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
#include <ben-bot/CLI.hpp>
#include <cstddef> // IWYU pragma: keep - for std::ptrdiff_t
#include <iterator>
#include <span>
#include <string>
#include <string_view>

namespace ben_bot {

using std::string_view;

namespace {
    [[nodiscard, gnu::cold]] std::string concat_strings(
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

Arguments Arguments::parse(
    const int argc, const char** argv)
{
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

    return Arguments {
        .noLoop     = check_for_arg("--no-loop"),
        .noLogo     = check_for_arg("--no-logo"),
        .uciCommand = concat_strings(args)
    };
}

} // namespace ben_bot
