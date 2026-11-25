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
#include <libbenbot/data-structures/ContinuationHistory.hpp>
#include <libbenbot/search/Context.hpp>
#include <libbenbot/search/Options.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/Move.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <memory>

inline constexpr auto TAGS { "[data-structures][ContinuationHistory]" };

using ben_bot::ContinuationHistory;
using chess::board::Square;
using chess::moves::Move;
using PieceType = chess::pieces::Type;

namespace {

constexpr auto MAX { ContinuationHistory::MAX_SCORE };

constexpr std::array allPieceTypes {
    PieceType::Pawn, PieceType::Knight, PieceType::Bishop,
    PieceType::Rook, PieceType::Queen, PieceType::King
};

[[nodiscard]] auto square(const std::size_t index) -> Square
{
    return Square::from_index(static_cast<chess::board::BitboardIndex>(index));
}

// calls the function with a move for every piece type & to-square. The table is indexed by
// piece type & to-square only, so the from-square doesn't matter (it only has to differ
// from the to-square)
template <typename Func>
void for_all_piece_tos(Func&& func)
{
    for (const auto type : allPieceTypes) {
        for (auto to = 0uz; to < 64uz; ++to) {
            func(Move { square(to == 0uz ? 1uz : 0uz), square(to), type });
        }
    }
}

// returns the number of entries in the table that aren't 0
[[nodiscard]] auto count_non_zero(const ContinuationHistory& table) -> std::size_t
{
    auto count { 0uz };

    for_all_piece_tos([&table, &count](const Move prev) {
        for_all_piece_tos([&table, &count, prev](const Move move) {
            for (const bool prevIsWhite : { true, false })
                for (const bool isWhite : { true, false })
                    if (table.get(prevIsWhite, prev, isWhite, move) != 0)
                        ++count;
        });
    });

    return count;
}

// this table is over 1 MiB, so it's put on the heap, like the search does
[[nodiscard]] auto make_table() -> std::unique_ptr<ContinuationHistory>
{
    return std::make_unique<ContinuationHistory>();
}

const Move whiteNf3 { square(6), square(21), PieceType::Knight }; // g1-f3
const Move whiteBf3 { square(6), square(21), PieceType::Bishop }; // same squares, different piece
const Move blackE5 { square(52), square(36), PieceType::Pawn };   // e7-e5
const Move blackD5 { square(51), square(35), PieceType::Pawn };   // d7-d5

} // namespace

TEST_CASE("Continuation history - a new table is empty", TAGS)
{
    const auto table = make_table();

    REQUIRE(count_non_zero(*table) == 0uz);
}

TEST_CASE("Continuation history - bonuses raise the score & penalties lower it", TAGS)
{
    const auto table = make_table();

    table->update(true, whiteNf3, false, blackE5, 400);

    const auto afterBonus = table->get(true, whiteNf3, false, blackE5);

    REQUIRE(afterBonus > 0);

    table->update(true, whiteNf3, false, blackD5, -400);

    REQUIRE(table->get(true, whiteNf3, false, blackD5) < 0);

    // a penalty after a bonus lowers the score again
    table->update(true, whiteNf3, false, blackE5, -100);

    REQUIRE(table->get(true, whiteNf3, false, blackE5) < afterBonus);
}

TEST_CASE("Continuation history - an update only changes its own entry", TAGS)
{
    const auto table = make_table();

    table->update(true, whiteNf3, false, blackE5, 400);

    REQUIRE(table->get(true, whiteNf3, false, blackE5) > 0);

    // the same moves, played by the other colors
    REQUIRE(table->get(true, whiteNf3, true, blackE5) == 0);
    REQUIRE(table->get(false, whiteNf3, false, blackE5) == 0);
    REQUIRE(table->get(false, whiteNf3, true, blackE5) == 0);

    // a different previous piece to the same square
    REQUIRE(table->get(true, whiteBf3, false, blackE5) == 0);

    // a different current move
    REQUIRE(table->get(true, whiteNf3, false, blackD5) == 0);

    REQUIRE(count_non_zero(*table) == 1uz);
}

TEST_CASE("Continuation history - moves are indexed by piece & to-square", TAGS)
{
    const auto table = make_table();

    // the from-square isn't part of the index
    const Move whiteNf3FromH2 { square(15), square(21), PieceType::Knight };

    table->update(true, whiteNf3, false, blackE5, 400);

    REQUIRE(table->get(true, whiteNf3FromH2, false, blackE5) == table->get(true, whiteNf3, false, blackE5));

    // a promotion is indexed as a pawn move
    const Move whiteE8Queen { square(52), square(60), PieceType::Pawn, PieceType::Queen };
    const Move whiteE8Pawn { square(52), square(60), PieceType::Pawn };
    const Move blackKe7 { square(60), square(52), PieceType::King };

    table->update(true, whiteE8Queen, false, blackKe7, 400);

    REQUIRE(table->get(true, whiteE8Pawn, false, blackKe7) > 0);
}

TEST_CASE("Continuation history - a null previous move has no entry", TAGS)
{
    const auto table = make_table();

    const Move nullMove { };

    for (const bool prevIsWhite : { true, false }) {
        for (const bool isWhite : { true, false }) {
            table->update(prevIsWhite, nullMove, isWhite, blackE5, 400);
            table->update(prevIsWhite, nullMove, isWhite, blackD5, -400);

            REQUIRE(table->get(prevIsWhite, nullMove, isWhite, blackE5) == 0);
            REQUIRE(table->get(prevIsWhite, nullMove, isWhite, blackD5) == 0);
        }
    }

    REQUIRE(count_non_zero(*table) == 0uz);
}

TEST_CASE("Continuation history - scores converge to the limit without passing it", TAGS)
{
    const auto table = make_table();

    for (auto i = 0; i < 10'000; ++i) {
        table->update(true, whiteNf3, false, blackE5, 1'000'000);
        table->update(true, whiteNf3, false, blackD5, -1'000'000);

        REQUIRE(table->get(true, whiteNf3, false, blackE5) <= MAX);
        REQUIRE(table->get(true, whiteNf3, false, blackD5) >= -MAX);
    }

    REQUIRE(table->get(true, whiteNf3, false, blackE5) == MAX);
    REQUIRE(table->get(true, whiteNf3, false, blackD5) == -MAX);
}

TEST_CASE("Continuation history - age() & clear()", TAGS)
{
    const auto table = make_table();

    table->update(true, whiteNf3, false, blackE5, 401);
    table->update(false, blackE5, true, whiteNf3, -401);

    const auto positive = table->get(true, whiteNf3, false, blackE5);
    const auto negative = table->get(false, blackE5, true, whiteNf3);

    REQUIRE(positive > 0);
    REQUIRE(negative < 0);

    table->age();

    REQUIRE(table->get(true, whiteNf3, false, blackE5) == positive / 2);
    REQUIRE(table->get(false, blackE5, true, whiteNf3) == negative / 2);

    table->clear();

    REQUIRE(count_non_zero(*table) == 0uz);
}

TEST_CASE("Search context - clear_history() clears the continuation history", TAGS)
{
    ben_bot::search::Context ctx;

    ctx.set_position(chess::game::Position { });

    ben_bot::search::Options options;
    options.depth = 6uz;

    ctx.set_options(options);

    // the setters above abort any in-progress search, so clear that before searching
    ctx.reset_abort();

    ctx.search();

    REQUIRE(count_non_zero(ctx.get_continuation_history()) > 0uz);

    ctx.clear_history();

    REQUIRE(count_non_zero(ctx.get_continuation_history()) == 0uz);
}
