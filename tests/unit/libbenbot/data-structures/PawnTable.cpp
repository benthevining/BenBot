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
#include <cstdint>
#include <libbenbot/data-structures/PawnTable.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libchess/board/Bitboard.hpp>

inline constexpr auto TAGS { "[data-structures][PawnTable]" };

using ben_bot::eval::TaperedScore;
using ben_bot::PawnEntry;
using ben_bot::PawnTable;

TEST_CASE("Pawn table - probe() indexing", TAGS)
{
    PawnTable table;

    static constexpr std::uint64_t key { UINT64_C(0x123456789abcdef0) };

    const auto* entry = &table.probe(key);

    // keys that differ only above the index bits share a slot
    REQUIRE(&table.probe(key + PawnTable::NumEntries) == entry);
    REQUIRE(&table.probe(key ^ (UINT64_C(1) << 63u)) == entry);

    // neighbouring keys don't
    REQUIRE(&table.probe(key + 1u) != entry);
    REQUIRE(&table.probe(key - 1u) != entry);
}

TEST_CASE("Pawn table - entries start empty & clear() empties them", TAGS)
{
    PawnTable table;

    static constexpr std::uint64_t key { UINT64_C(0xfedcba9876543210) };

    REQUIRE(table.probe(key).key == 0u);
    REQUIRE(table.probe(key).passers.none());
    REQUIRE(table.probe(key).score == TaperedScore { });

    table.probe(key) = PawnEntry {
        .key     = key,
        .passers = chess::board::Bitboard { UINT64_C(0x0000100000000800) },
        .score   = TaperedScore { .mg = -42, .eg = 17 }
    };

    REQUIRE(table.probe(key).key == key);
    REQUIRE(table.probe(key).passers.count() == 2uz);
    REQUIRE(table.probe(key).score == TaperedScore { .mg = -42, .eg = 17 });

    table.clear();

    REQUIRE(table.probe(key).key == 0u);
    REQUIRE(table.probe(key).passers.none());
    REQUIRE(table.probe(key).score == TaperedScore { });
}
