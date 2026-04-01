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

namespace util::strings {

using std::size_t;

auto TextTable::append_column(const string_view text) -> TextTable&
{
    if (std::exchange(startNewRow, false))
        rows.emplace_back();

    rows.back().add_column(text);

    return *this;
}

auto TextTable::new_row() -> TextTable&
{
    if (std::exchange(startNewRow, true))
        rows.emplace_back();

    return *this;
}

namespace {
    inline constexpr string_view COLUMN_SEPARATOR { " | " };
    inline constexpr string_view LINE_START { "| " };
    inline constexpr string_view LINE_ENDING { " |" };

    void add_header_sep_row(
        const TextTable::Widths widths, string& output)
    {
        output.append(LINE_START);

        std::ranges::for_each(
            widths.first(widths.size() - 1uz),
            [&output](const size_t width) {
                output.append(width, '-');
                output.append(COLUMN_SEPARATOR);
            });

        output.append(widths.back(), '-');
        output.append(LINE_ENDING);
    }

    [[nodiscard]] auto make_header_sep_row(
        const TextTable::Widths widths) -> string
    {
        string result;
        add_header_sep_row(widths, result);
        return result;
    }
} // namespace

auto TextTable::to_string() const -> string
{
    const auto widths = get_column_widths();

    auto result = rows.front().to_string(widths);
    result.append(1uz, '\n');

    add_header_sep_row(widths, result);

    result.append(1uz, '\n');

    std::ranges::for_each(
        rows | std::views::drop(1uz),
        [&result, &widths](const Row& row) {
            result.append(row.to_string(widths));
            result.append(1uz, '\n');
        });

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
        printHeading, printOutline, widths);

    printNewline();

    printOutline(make_header_sep_row(widths));

    printNewline();

    std::ranges::for_each(
        rows | std::views::drop(1uz),
        [cell       = std::move(printCell),
            outline = std::move(printOutline),
            newLine = std::move(printNewline),
            &widths](const Row& row) {
            row.print(cell, outline, widths);
            newLine();
        });
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
    const Widths widths) const
    -> string
{
    string result { LINE_START };

    auto index { 0uz };

    for (const auto width : widths) {
        if (index > 0uz)
            result.append(COLUMN_SEPARATOR);

        const auto colLen = [this, index, &result] {
            if (index < columns.size()) {
                const auto& colText = columns.at(index);
                result.append(colText);
                return colText.length();
            }

            return 0uz;
        }();

        result.append(width - colLen, ' ');

        ++index;
    }

    result.append(LINE_ENDING);

    return result;
}

void TextTable::Row::print(
    const PrintFunc&        printCell,
    const PrintFunc&        printOutline,
    std::span<const size_t> widths) const
{
    printOutline(LINE_START);

    string padded;

    auto index { 0uz };

    for (const auto width : widths) {
        if (index > 0uz)
            printOutline(COLUMN_SEPARATOR);

        if (index < columns.size())
            padded = columns.at(index);
        else
            padded.clear();

        padded.resize(width, ' ');

        printCell(padded);

        ++index;
    }

    printOutline(LINE_ENDING);
}

} // namespace util::strings
