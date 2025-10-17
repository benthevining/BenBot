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
#include <ben-bot/ColorPrinting.hpp>
#include <ben-bot/Engine.hpp>
#include <ben-bot/Resources.hpp>
#include <ben-bot/TextTable.hpp>
#include <cassert>
#include <iostream>
#include <libchess/game/Position.hpp>
#include <libchess/util/Strings.hpp>
#include <ranges>
#include <string_view>
#include <termcolor/termcolor.hpp> // this is the only TU that includes the termcolor library

namespace ben_bot {

using std::string_view;

void Engine::print_logo_and_version() const
{
    using Lines = beman::inplace_vector::inplace_vector<string_view, 11uz>;

    const auto logoLines = chess::util::lines_view(resources::get_ascii_logo())
                         | std::views::take(Lines::capacity())
                         | std::ranges::to<Lines>();

    assert(logoLines.size() == Lines::capacity());

    std::cout << termcolor::grey << logoLines.front() << '\n'
              << termcolor::blue;

    for (const auto line : logoLines | std::views::drop(1) | std::views::take(Lines::capacity() - 2uz))
        std::cout << line << '\n';

    std::cout << termcolor::grey << logoLines.back() << "\n\n"
              << termcolor::reset << termcolor::bold << get_name() << ", "
              << termcolor::reset << "by " << get_author() << '\n'
              << termcolor::reset;
}

void print_colored_table(const TextTable& table)
{
    table.print(
        [](const string_view heading) {
            // we want the heading text to be underlined, but not the
            // whitespace that follows the text to complete the cell
            const auto trimmed = chess::util::trim(heading);

            std::cout << termcolor::bold << termcolor::underline
                      << trimmed
                      << termcolor::reset;

            const auto numSpaces = heading.length() - trimmed.length();

            for (auto i = 0uz; i < numSpaces; ++i)
                std::cout << ' ';
        },
        [](const string_view cell) {
            std::cout << cell;
        },
        [](const string_view outline) {
            std::cout << termcolor::white << outline << termcolor::reset;
        },
        [] { std::cout << '\n'; });

    std::cout << termcolor::reset;
}

void print_colored_board(const Position& pos, const bool utf8)
{
    const auto boardStr = utf8 ? print_utf8(pos) : print_ascii(pos);

    using Lines = beman::inplace_vector::inplace_vector<string_view, 9uz>;

    const auto lines = chess::util::lines_view(boardStr)
                     | std::ranges::to<Lines>();

    assert(lines.size() == Lines::capacity());

    for (const auto line : lines | std::views::take(Lines::capacity() - 1uz)) {
        std::cout << line.substr(0uz, line.length() - 1uz)
                  << termcolor::white << line.back() << '\n'
                  << termcolor::reset;
    }

    std::cout << termcolor::white << lines.back() << '\n'
              << termcolor::reset;
}

void print_labeled_info(const string_view label, const string_view info)
{
    std::cout << termcolor::white << label << termcolor::reset << info << '\n';
}

} // namespace ben_bot
