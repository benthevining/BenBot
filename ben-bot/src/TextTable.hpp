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

#pragma once

#include <algorithm>
#include <cstddef> // IWYU pragma: keep - for size_t;
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ben_bot {

using std::size_t;
using std::string;
using std::string_view;

// helps to create tables of text with vertically aligned columns
// rows can have a different number of columns; the table's size is based on the maximum number of columns
struct TextTable final {
    // appends a column to the current row
    TextTable& append_column(string_view text);

    // ends the current row
    // subsequent calls to append_column() will write to the new row
    TextTable& new_row();

    [[nodiscard]] size_t num_columns() const;

    // returns a set of strings, one for each row, which have been
    // padded and formatted to align vertically
    [[nodiscard]] std::vector<string> get_rows(
        string_view rowPrefix,
        string_view columnSeparator,
        string_view rowSuffix) const;

    // concatenates all the rows in the table into a single string
    [[nodiscard]] string to_string(
        string_view rowPrefix,
        string_view columnSeparator,
        string_view rowSuffix) const;

private:
    struct Row final {
        void add_column(string_view text);

        [[nodiscard]] std::span<const string> get_columns() const noexcept { return columns; }

        [[nodiscard]] string to_string(
            string_view             columnSeparator,
            std::span<const size_t> widths) const;

    private:
        std::vector<string> columns;
    };

    [[nodiscard]] std::vector<size_t> get_column_widths() const;

    std::vector<Row> rows;

    bool startNewRow { true };
};

/*
                         ___                           ,--,
      ,---,            ,--.'|_                ,--,   ,--.'|
    ,---.'|            |  | :,'             ,--.'|   |  | :
    |   | :            :  : ' :             |  |,    :  : '    .--.--.
    |   | |   ,---.  .;__,'  /    ,--.--.   `--'_    |  ' |   /  /    '
  ,--.__| |  /     \ |  |   |    /       \  ,' ,'|   '  | |  |  :  /`./
 /   ,'   | /    /  |:__,'| :   .--.  .-. | '  | |   |  | :  |  :  ;_
.   '  /  |.    ' / |  '  : |__  \__\/: . . |  | :   '  : |__ \  \    `.
'   ; |:  |'   ;   /|  |  | '.'| ," .--.; | '  : |__ |  | '.'| `----.   \
|   | '/  ''   |  / |  ;  :    ;/  /  ,.  | |  | '.'|;  :    ;/  /`--'  /__  ___  ___
|   :    :||   :    |  |  ,   /;  :   .'   \;  :    ;|  ,   /'--'.     /  .\/  .\/  .\
 \   \  /   \   \  /    ---`-' |  ,     .-./|  ,   /  ---`-'   `--'---'\  ; \  ; \  ; |
  `----'     `----'             `--`---'     ---`-'                     `--" `--" `--"

 */

inline TextTable& TextTable::append_column(const string_view text)
{
    if (startNewRow) {
        rows.emplace_back();
        startNewRow = false;
    }

    rows.back().add_column(text);

    return *this;
}

inline TextTable& TextTable::new_row()
{
    if (startNewRow)
        rows.emplace_back();
    else
        startNewRow = true;

    return *this;
}

inline std::vector<string> TextTable::get_rows(
    const string_view rowPrefix,
    const string_view columnSeparator,
    const string_view rowSuffix) const
{
    std::vector<string> result;

    result.reserve(rows.size());

    const auto widths = get_column_widths();

    for (const auto& row : rows) {
        result.emplace_back(
            std::format(
                "{}{}{}",
                rowPrefix, row.to_string(columnSeparator, widths), rowSuffix));
    }

    return result;
}

inline string TextTable::to_string(
    const string_view rowPrefix,
    const string_view columnSeparator,
    const string_view rowSuffix) const
{
    string result;

    for (const auto& row : get_rows(rowPrefix, columnSeparator, rowSuffix))
        result.append(row);

    return result;
}

inline size_t TextTable::num_columns() const
{
    size_t maxColummns { 0uz };

    for (const auto& row : rows)
        maxColummns = std::max(maxColummns, row.get_columns().size());

    return maxColummns;
}

inline std::vector<size_t> TextTable::get_column_widths() const
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

inline void TextTable::Row::add_column(const string_view text)
{
    columns.emplace_back(text);
}

inline string TextTable::Row::to_string(
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
