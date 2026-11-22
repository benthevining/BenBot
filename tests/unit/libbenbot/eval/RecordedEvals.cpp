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

#include "eval/RecordedEvals.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <cstdint>
#include <libbenbot/eval/Evaluation.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>

// This checks that the eval gives exactly the same results as when these values were recorded,
// for the bench.epd positions & the positions in the other eval tests, plus a few endgames.
// It's meant for refactors that mustn't change the eval. If you change the eval on purpose,
// record the new values instead. The values are in RecordedEvals.hpp.

namespace {

inline constexpr auto TREE_DEPTH = 2uz;

// FNV-1a over the evals, in move generation order
void hash_tree( // NOLINT(misc-no-recursion)
    const chess::game::Position& position, const std::size_t depth, std::uint64_t& hash)
{
    const auto eval = static_cast<std::int16_t>(ben_bot::eval::evaluate(position));

    hash ^= static_cast<std::uint16_t>(eval);
    hash *= UINT64_C(1099511628211);

    if (depth == 0uz)
        return;

    for (const auto& move : chess::moves::generate(position))
        hash_tree(after_move(position, move), depth - 1uz, hash);
}

} // namespace

TEST_CASE("Evaluation - unchanged from recorded values", "[eval][evaluation]")
{
    for (const auto& expected : recorded_evals::EXPECTED_EVALS) {
        INFO(expected.fen);

        const auto position = chess::notation::from_fen(expected.fen).value();

        REQUIRE(static_cast<int>(ben_bot::eval::evaluate(position)) == expected.eval);

        std::uint64_t hash { UINT64_C(14695981039346656037) };

        hash_tree(position, TREE_DEPTH, hash);

        REQUIRE(hash == expected.treeHash);
    }
}
