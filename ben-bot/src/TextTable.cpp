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

#include "TextTable.hpp"
#include <algorithm>
#include <format>
#include <span>
#include <string>
#include <vector>

namespace ben_bot {

TextTable& TextTable::append_column(const string_view text)
{
    if (startNewRow) {
        rows.emplace_back();
        startNewRow = false;
    }

    rows.back().add_column(text);

    return *this;
}

TextTable& TextTable::new_row()
{
    if (startNewRow)
        rows.emplace_back();
    else
        startNewRow = true;

    return *this;
}

string TextTable::to_string(
    const string_view rowPrefix,
    const string_view columnSeparator,
    const string_view rowSuffix) const
{
    const auto widths = get_column_widths();

    string result;

    for (const auto& row : rows) {
        result.append(
            std::format(
                "{}{}{}",
                rowPrefix, row.to_string(columnSeparator, widths), rowSuffix));
    }

    return result;
}

size_t TextTable::num_columns() const
{
    size_t maxColummns { 0uz };

    for (const auto& row : rows)
        maxColummns = std::max(maxColummns, row.get_columns().size()); // cppcheck-suppress useStlAlgorithm

    return maxColummns;
}

std::vector<size_t> TextTable::get_column_widths() const
{
    std::vector<size_t> widths;

    widths.resize(num_columns());

    for (const auto& row : rows) {
        const auto columns = row.get_columns();

        for (auto i = 0uz; i < columns.size(); ++i)
            widths[i] = std::max(widths[i], columns[i].length());
    }

    return widths;
}

string TextTable::Row::to_string(
    const string_view             columnSeparator,
    const std::span<const size_t> widths) const
{
    string result;

    size_t index { 0uz };

    for (const auto width : widths) {
        if (index > 0uz)
            result.append(columnSeparator);

        string padded;

        if (index < columns.size())
            padded = columns[index];

        padded.resize(width, ' ');

        result.append(padded);

        ++index;
    }

    return result;
}

} // namespace ben_bot
