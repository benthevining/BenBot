/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <cassert>
#include <cctype> // IWYU pragma: keep - for std::isspace()
#include <format>
#include <libchess/util/Strings.hpp>
#include <stdexcept>
#include <string_view>

namespace {

// NB. not [[gnu::const]] because std::isspace() depends on the current C locale
[[nodiscard]] bool is_whitespace(const char text) noexcept
{
    return std::isspace(static_cast<unsigned char>(text)) != 0;
}

[[nodiscard]] std::string_view trim_start(std::string_view text) noexcept
{
    auto idx { 0uz };

    for (const auto letter : text) {
        if (! is_whitespace(letter)) {
            text.remove_prefix(idx);
            return text; // NOLINT
        }

        ++idx;
    }

    return {};
}

[[nodiscard]] std::string_view trim_end(const std::string_view text)
{
    if (text.empty())
        return {};

    for (auto i = text.length(); i > 0uz; --i)
        if (! is_whitespace(text[i - 1uz]))
            return text.substr(0uz, i);

    return {};
}

} // namespace

namespace chess::util {

std::string_view trim(const std::string_view text)
{
    return trim_start(trim_end(text));
}

size_t find_matching_close_paren(const std::string_view input)
{
    assert(input.front() == '(');

    size_t numOpenParens { 1uz };
    size_t numCloseParens { 0uz };

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
            }

            default: continue;
        }
    }

    throw std::invalid_argument {
        std::format("Unmatched ( in input string: '{}'", input)
    };
}

} // namespace chess::util
