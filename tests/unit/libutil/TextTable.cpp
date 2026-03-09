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

#include <catch2/catch_test_macros.hpp>
#include <libutil/TextTable.hpp>
#include <string>
#include <string_view>

static constexpr auto TAGS { "[util][strings][TextTable]" };

using util::strings::TextTable;

namespace {
[[nodiscard]] auto get_printed_string(const TextTable& table) -> std::string
{
    std::string result;

    auto appendToString = [&result](const std::string_view toAdd) {
        result.append(toAdd);
    };

    table.print(
        appendToString, appendToString, appendToString,
        [&result] { result.append(1uz, '\n'); });

    return result;
}
} // namespace

TEST_CASE("TextTable - to_string() should give same result as print()", TAGS)
{
    TextTable table;

    table.append_column("Option")
        .append_column("Type")
        .append_column("Notes")
        .append_column("Default");

    table.new_row()
        .append_column("Option 1")
        .append_column("Integer")
        .append_column("Comments")
        .append_column("0");

    table.new_row()
        .append_column("Option 2")
        .append_column("String")
        .append_column("More comments")
        .append_column("foo");

    REQUIRE(
        get_printed_string(table) == table.to_string());
}
