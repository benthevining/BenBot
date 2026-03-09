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
#include <cstddef> // IWYU pragma: keep - for size_t
#include <functional>
#include <libutil/TextTable.hpp>
#include <numeric>
#include <ranges>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace chess::util::strings {

using std::size_t;

auto TextTable::append_column(const string_view text) -> TextTable&
{
    if (startNewRow) {
        rows.emplace_back();
        startNewRow = false;
    }

    rows.back().add_column(text);

    return *this;
}

auto TextTable::new_row() -> TextTable&
{
    if (startNewRow)
        rows.emplace_back();
    else
        startNewRow = true;

    return *this;
}

namespace {
    inline constexpr string_view COLUMN_SEPARATOR { " | " };
    inline constexpr string_view LINE_START { "| " };
    inline constexpr string_view LINE_ENDING { " |" };

    [[nodiscard, gnu::cold]] auto make_header_sep_row(
        const std::span<const size_t> widths)
        -> string
    {
        string result { LINE_START };

        for (const auto width : widths.first(widths.size() - 1uz)) {
            result.append(width, '-');
            result.append(COLUMN_SEPARATOR);
        }

        result.append(widths.back(), '-');
        result.append(LINE_ENDING);

        return result;
    }
} // namespace

auto TextTable::to_string() const -> string
{
    const auto widths = get_column_widths();

    auto result = rows.front().to_string(widths);
    result.append(1uz, '\n');

    result.append(make_header_sep_row(widths));
    result.append(1uz, '\n');

    for (const auto& row : rows | std::views::drop(1uz)) {
        result.append(row.to_string(widths));
        result.append(1uz, '\n');
    }

    return result;
}

void TextTable::print(
    PrintFunc&&           printHeading,
    PrintFunc&&           printCell,
    PrintFunc&&           printOutline,
    std::function<void()> printNewline) const
{
    const auto widths = get_column_widths();

    rows.front().print(
        std::move(printHeading), printOutline, widths);

    printNewline();

    printOutline(make_header_sep_row(widths));

    printNewline();

    for (const auto& row : rows | std::views::drop(1uz)) {
        row.print(printCell, printOutline, widths);
        printNewline();
    }
}

auto TextTable::num_columns() const -> size_t
{
    return std::transform_reduce(
        rows.begin(), rows.end(),
        0uz,
        [](const size_t first, const size_t second) { return std::max(first, second); },
        [](const Row& row) { return row.get_columns().size(); });
}

auto TextTable::get_column_widths() const -> std::vector<size_t>
{
    std::vector<size_t> widths;

    widths.resize(num_columns());

    for (const auto& row : rows) {
        const auto columns = row.get_columns();

        for (auto i = 0uz; i < columns.size(); ++i)
            widths.at(i) = std::max(widths.at(i), columns[i].length());
    }

    return widths;
}

auto TextTable::Row::to_string(
    const std::span<const size_t> widths) const
    -> string
{
    string result { LINE_START };

    auto index { 0uz };

    for (const auto width : widths) {
        if (index > 0uz)
            result.append(COLUMN_SEPARATOR);

        string padded;

        if (index < columns.size())
            padded = columns.at(index);

        padded.resize(width, ' ');

        result.append(padded);

        ++index;
    }

    result.append(LINE_ENDING);

    return result;
}

void TextTable::Row::print(
    PrintFunc               printCell,
    PrintFunc               printOutline,
    std::span<const size_t> widths) const
{
    printOutline(LINE_START);

    auto index { 0uz };

    for (const auto width : widths) {
        if (index > 0uz)
            printOutline(COLUMN_SEPARATOR);

        string padded;

        if (index < columns.size())
            padded = columns[index];

        padded.resize(width, ' ');

        printCell(padded);

        ++index;
    }

    printOutline(LINE_ENDING);
}

} // namespace chess::util::strings
