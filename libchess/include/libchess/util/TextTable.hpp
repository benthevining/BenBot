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
    This file defines the TextTable utility class.
    @ingroup strings
 */

#pragma once

#include <cstddef> // IWYU pragma: keep - for size_t
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace chess::util::strings {

using std::size_t;
using std::string;
using std::string_view;

/** This class allows you to construct a table of text with vertically aligned columns.
    Rows may have different numbers of columns; the table's size is based on the maximum number of columns.
    The first row is the header, and a separator row will be added between it and the second manually added row.

    @ingroup strings
 */
struct [[nodiscard]] TextTable final {
    /** Appends a column to the current row. */
    auto append_column(string_view text) -> TextTable&;

    /** Ends the current row.
        Subsequent calls to ``append_column()`` will write to the new row.
     */
    auto new_row() -> TextTable&;

    /** Concatenates all the rows in the table into a single string, with separators between rows and columns. */
    [[nodiscard]] auto to_string() const -> string;

private:
    struct Row final {
        void add_column(string_view text) { columns.emplace_back(text); }

        [[nodiscard]] auto get_columns() const noexcept -> std::span<const string>
        {
            return columns;
        }

        [[nodiscard]] auto to_string(std::span<const size_t> widths) const -> string;

    private:
        std::vector<string> columns;
    };

    [[nodiscard]] auto num_columns() const -> size_t;

    [[nodiscard]] auto get_column_widths() const -> std::vector<size_t>;

    std::vector<Row> rows;

    bool startNewRow { true };
};

} // namespace chess::util::strings
