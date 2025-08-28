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

#include <cstddef> // IWYU pragma: keep - for size_t
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
// the first row is the header, and a separator row will be added between it and the second manually added row
struct TextTable final {
    // appends a column to the current row
    TextTable& append_column(string_view text);

    // ends the current row
    // subsequent calls to append_column() will write to the new row
    TextTable& new_row();

    // concatenates all the rows in the table into a single string
    [[nodiscard]] string to_string() const;

private:
    struct Row final {
        void add_column(string_view text) { columns.emplace_back(text); }

        [[nodiscard]] std::span<const string> get_columns() const noexcept { return columns; }

        [[nodiscard]] string to_string(std::span<const size_t> widths) const;

    private:
        std::vector<string> columns;
    };

    [[nodiscard]] size_t num_columns() const;

    [[nodiscard]] std::vector<size_t> get_column_widths() const;

    std::vector<Row> rows;

    bool startNewRow { true };
};

} // namespace ben_bot
