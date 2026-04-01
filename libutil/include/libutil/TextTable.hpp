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
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace util::strings {

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

    /** Typedef for a function that accepts a string as an argument. */
    using PrintFunc = std::function<void(string_view)>;

    /** Prints the table by calling lambda functions for each different element of the table.
        This allows you to do things like print the table to a terminal using ANSI color codes; using
        just the ``to_string()`` API would make it much more difficult to intersperse the needed escape
        codes to switch between styles for the cell text and outlines, etc.

        @param printHeading Function object that will be called to print the contents of each heading cell.
        Note that the strings sent to this function will include trailing whitespace, to create the table's alignment.

        @param printCell Function object that will be called to print the contents of each non-heading cell.
        Note that the strings sent to this function will include trailing whitespace, to create the table's alignment.

        @param printOutline Function object that will be called to print outline/border characters between cells
        and surrounding the entire table.

        @param printNewline Function object that accepts no arguments and should insert a single newline character
        into the text stream being constructed.
     */
    void print(
        PrintFunc&&           printHeading,
        PrintFunc&&           printCell,
        PrintFunc&&           printOutline,
        std::function<void()> printNewline) const;

    /** @internal */
    using Widths = std::span<const size_t>;

private:
    struct Row final {
        void add_column(string_view text) { columns.emplace_back(text); }

        [[nodiscard]] auto get_columns() const noexcept -> std::span<const string>
        {
            return columns;
        }

        [[nodiscard]] auto to_string(Widths widths) const -> string;

        void print(
            const PrintFunc&        printCell,
            const PrintFunc&        printOutline,
            std::span<const size_t> widths) const;

    private:
        std::vector<string> columns;
    };

    [[nodiscard]] auto num_columns() const -> size_t;

    [[nodiscard]] auto get_column_widths() const -> std::vector<size_t>;

    std::vector<Row> rows;

    bool startNewRow { true };
};

} // namespace util::strings
