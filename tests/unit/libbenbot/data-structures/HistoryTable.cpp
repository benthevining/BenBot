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

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstdlib>
#include <libbenbot/data-structures/HistoryGravity.hpp>
#include <libbenbot/data-structures/HistoryTable.hpp>
#include <libbenbot/search/Callbacks.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/PieceTypes.hpp>

inline constexpr auto TAGS { "[data-structures][HistoryTable]" };

using ben_bot::apply_history_gravity;
using ben_bot::HistoryTable;
using chess::board::Square;
using chess::moves::Move;
using PieceType = chess::pieces::Type;

namespace {

constexpr auto MAX { HistoryTable::MAX_SCORE };

// calls the function with a move for every from & to square pair. The table is indexed
// by from & to square only, so the piece type doesn't matter
template <typename Func>
void for_all_squares(Func&& func)
{
    for (auto from = 0uz; from < 64uz; ++from) {
        for (auto to = 0uz; to < 64uz; ++to) {
            func(Move {
                Square::from_index(static_cast<chess::board::BitboardIndex>(from)),
                Square::from_index(static_cast<chess::board::BitboardIndex>(to)),
                PieceType::Knight });
        }
    }
}

[[nodiscard]] auto is_empty(const HistoryTable& table) -> bool
{
    bool empty = true;

    for_all_squares([&table, &empty](const Move move) {
        empty = empty and table.get(true, move) == 0 and table.get(false, move) == 0;
    });

    return empty;
}

} // namespace

TEST_CASE("History gravity - repeated bonuses converge to the limit without passing it", TAGS)
{
    for (const auto bonus : { MAX, 1000, 1 }) {
        std::int16_t positive { 0 };
        std::int16_t negative { 0 };

        for (auto i = 0; i < 100'000; ++i) {
            apply_history_gravity(positive, bonus, MAX);
            apply_history_gravity(negative, -bonus, MAX);

            REQUIRE(positive <= MAX);
            REQUIRE(negative >= -MAX);
        }

        REQUIRE(positive == MAX);
        REQUIRE(negative == -MAX);
    }
}

TEST_CASE("History gravity - a bonus beyond the limit is clamped", TAGS)
{
    for (const int start : { -MAX, -5000, 0, 123, MAX }) {
        std::int16_t clamped    = static_cast<std::int16_t>(start);
        std::int16_t atTheLimit = static_cast<std::int16_t>(start);

        apply_history_gravity(clamped, 1'000'000, MAX);
        apply_history_gravity(atTheLimit, MAX, MAX);

        REQUIRE(clamped == atTheLimit);
        REQUIRE(clamped == MAX);

        clamped    = static_cast<std::int16_t>(start);
        atTheLimit = static_cast<std::int16_t>(start);

        apply_history_gravity(clamped, -1'000'000, MAX);
        apply_history_gravity(atTheLimit, -MAX, MAX);

        REQUIRE(clamped == atTheLimit);
        REQUIRE(clamped == -MAX);
    }
}

TEST_CASE("History gravity - scores never leave the allowed range", TAGS)
{
    static constexpr std::array bonuses {
        -1'000'000, -MAX, -MAX + 1, -9999, -256, -16, -1, 0, 1, 16, 256, 9999, MAX - 1, MAX, 1'000'000
    };

    for (auto start = -MAX; start <= MAX; ++start) {
        for (const auto bonus : bonuses) {
            std::int16_t entry = static_cast<std::int16_t>(start);

            apply_history_gravity(entry, bonus, MAX);

            REQUIRE(entry >= -MAX);
            REQUIRE(entry <= MAX);

            // a bonus never makes the score go down, & a penalty never makes it go up
            if (bonus > 0)
                REQUIRE(entry >= start);
            else if (bonus < 0)
                REQUIRE(entry <= start);
        }
    }
}

TEST_CASE("History gravity - int16 & int entries give the same results", TAGS)
{
    for (auto start = -MAX; start <= MAX; start += 7) {
        for (const auto bonus : { -MAX, -300, -1, 1, 300, MAX }) {
            std::int16_t small = static_cast<std::int16_t>(start);
            int          large = start;

            apply_history_gravity(small, bonus, MAX);
            apply_history_gravity(large, bonus, MAX);

            REQUIRE(small == large);

            // this is the formula that HistoryTable::update() used before the helper existed
            REQUIRE(large == start + bonus - (start * std::abs(bonus) / MAX));
        }
    }
}

TEST_CASE("History table - update(), age() & clear()", TAGS)
{
    HistoryTable table;

    REQUIRE(is_empty(table));

    const Move e2e4 { Square::from_index(12), Square::from_index(28), PieceType::Pawn };
    const Move g1f3 { Square::from_index(6), Square::from_index(21), PieceType::Knight };

    table.update(true, e2e4, 400);

    REQUIRE(table.get(true, e2e4) > 0);
    REQUIRE(table.get(false, e2e4) == 0); // the other side's score isn't changed
    REQUIRE(table.get(true, g1f3) == 0);

    table.update(false, g1f3, -400);

    REQUIRE(table.get(false, g1f3) < 0);
    REQUIRE(table.get(true, g1f3) == 0);

    const auto white = table.get(true, e2e4);
    const auto black = table.get(false, g1f3);

    table.age();

    REQUIRE(table.get(true, e2e4) == white / 2);
    REQUIRE(table.get(false, g1f3) == black / 2);

    table.clear();

    REQUIRE(is_empty(table));
}

TEST_CASE("Search context - clear_history() clears the history table", TAGS)
{
    ben_bot::search::Context ctx;

    ctx.set_position(chess::game::Position { });

    ben_bot::search::Options options;
    options.depth = 6uz;

    ctx.set_options(options);

    // the setters above abort any in-progress search, so clear that before searching
    ctx.reset_abort();

    ctx.search();

    REQUIRE(not is_empty(ctx.get_history_table()));

    ctx.clear_history();

    REQUIRE(is_empty(ctx.get_history_table()));
}
