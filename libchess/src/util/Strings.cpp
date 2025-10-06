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
#include <cassert>
#include <cctype>  // IWYU pragma: keep - for std::isspace()
#include <cstddef> // IWYU pragma: keep - for size_t
#include <expected>
#include <format>
#include <iterator>
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <string>
#include <string_view>

namespace {

using std::size_t;
using std::string_view;

[[nodiscard]] auto is_non_whitespace(const char text) noexcept -> bool
{
    // this should take care of \r\n sequences on Windows
    return std::isspace(static_cast<unsigned char>(text)) == 0;
}

[[nodiscard]] auto trim_start(string_view text) noexcept -> string_view // NOLINT(bugprone-exception-escape)
{
    return text.substr(
        static_cast<size_t>(
            std::ranges::distance(
                text.begin(),
                std::ranges::find_if(text, is_non_whitespace))));
}

[[nodiscard]] auto trim_end(const string_view text) -> string_view
{
    return text.substr(0uz,
        static_cast<size_t>(
            std::ranges::distance(
                text.begin(),
                std::ranges::find_if(
                    std::views::reverse(text), is_non_whitespace)
                    .base())));
}

} // namespace

namespace chess::util {

auto trim(const string_view text) -> string_view
{
    return trim_start(trim_end(text));
}

auto find_matching_close_paren(const string_view input) -> std::expected<size_t, std::string>
{
    assert(input.front() == '(');

    auto numOpenParens { 1uz };
    auto numCloseParens { 0uz };

    for (auto idx = 1uz; idx < input.size(); ++idx) {
        switch (input[idx]) {
            case '(': {
                ++numOpenParens;
                continue;
            }

            case ')': {
                ++numCloseParens;

                if (numOpenParens == numCloseParens)
                    return idx;

                continue;
            }

            default: continue;
        }
    }

    return std::unexpected {
        std::format(
            "Unmatched ( in input string: '{}'",
            input)
    };
}

auto split_at_first_space(const string_view input) -> StringViewPair
{
    const auto spaceIdx = input.find(' ');

    if (spaceIdx == string_view::npos)
        return { input, {} };

    return {
        input.substr(0uz, spaceIdx),
        input.substr(spaceIdx + 1uz)
    };
}

auto split_at_first_space_or_newline(const string_view input) -> StringViewPair
{
    // TODO: will \r\n sequences on Windows mess this up?
    const auto firstDelimIdx = std::min(
        input.find(' '),
        input.find('\n'));

    if (firstDelimIdx == string_view::npos)
        return { input, {} };

    return {
        input.substr(0uz, firstDelimIdx),
        input.substr(firstDelimIdx + 1uz)
    };
}

} // namespace chess::util
