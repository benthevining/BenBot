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

/** @file
    This file defines the TextTable utility.
    @ingroup benbot
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

/** This class allows you to construct a table of text with vertically aligned columns.
    Rows may have different numbers of columns; the table's size is based on the maximum number of columns.
    The first row is the header, and a separator row will be added between it and the second manually added row.

    @ingroup benbot
 */
struct TextTable final {
    /** Appends a column to the current row. */
    TextTable& append_column(string_view text);

    /** Ends the current row.
        Subsequent calls to ``append_column()`` will write to the new row.
     */
    TextTable& new_row();

    /** Concatenates all the rows in the table into a single string, with separators between rows and columns. */
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
