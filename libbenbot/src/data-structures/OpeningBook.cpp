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

#include <cassert>
#include <libbenbot/data-structures/OpeningBook.hpp>
#include <libchess/notation/UCI.hpp>
#include <libchess/util/Strings.hpp>
#include <nlohmann/json.hpp>
#include <string_view>
#include <vector>

namespace chess::search {

void OpeningBook::add_line(std::string_view line)
{
    Position position;

    while (! line.empty()) {
        auto [first, rest] = util::split_at_first_space(line);

        first = util::trim(first);

        const auto move = notation::from_uci(position, first);

        lines[position.hash].emplace_back(move);

        position.make_move(move);

        line = rest;
    }
}

/* Example JSON:

    [
        {
            "comment": "Four knights",
            "lines": [
                "e2e4 e7e5 b1c3 g8f6 g1f3 b8c6 f1b5 f8b4 e1g1 e8g8 d2d3 d7d6 c1g5 b4c3 b2c3 d8e7 f1e1 c6d8 d3d4 d8e6"
            ]
        },
        {
            "comment": "Italian",
            "lines": [
                "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d4 e5d4 c3d4 c5b4 c1d2 b4d2 b1d2 d7d5",
                "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d3 d7d6 b2b4-c5b6 a2a4 a7a5 b4b5 c6e7",
                "e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 d2d3 g8f6 c2c3 d7d6 c4b3 a7a6 b1d2 c5a7 h2h3 c6e7"
            ]
        }
    ]

    Comment is optional, but lines is required.
 */
void OpeningBook::add_from_json(const std::string_view json)
{
    const auto data = nlohmann::json::parse(json);

    assert(data.is_array());

    std::vector<std::string_view> lines;

    for (const auto& child : data) {
        lines.clear();

        child.at("lines").get_to(lines);

        for (const auto line : lines)
            add_line(line);
    }
}

} // namespace chess::search
