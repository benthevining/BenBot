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
#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <iterator>
#include <libbenbot/Resources.hpp>
#include <libbenbot/data-structures/PawnTable.hpp>
#include <libbenbot/eval/DefaultWeights.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/TaperedScore.hpp>
#include <libbenbot/eval/Trace.hpp>
#include <libbenbot/eval/Weights.hpp>
#include <libchess/board/BitboardIndex.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/EPD.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/pieces/Colors.hpp>
#include <libchess/pieces/PieceTypes.hpp>
#include <utility>
#include <vector>

// These check that the eval can be rebuilt exactly from its trace & the weights,
// which is what a tuner relies on.

namespace {

using ben_bot::eval::CoeffTracer;
using ben_bot::eval::DEFAULT_WEIGHTS;
using ben_bot::eval::evaluate;
using ben_bot::eval::evaluate_traced;
using ben_bot::eval::offset_of;
using ben_bot::eval::TaperedScore;
using ben_bot::eval::Term;
using chess::game::Position;
using chess::pieces::Color;
using std::size_t;

// the bench.epd positions, plus the positions that RecordedEvals checks
[[nodiscard]] auto get_test_positions() -> std::vector<Position>
{
    std::vector<Position> positions;

    std::ranges::transform(
        chess::notation::parse_all_epds(ben_bot::resources::get_bench_epd_text()),
        std::back_inserter(positions),
        [](const auto& epd) { return epd.position; });

    std::ranges::transform(
        recorded_evals::EXPECTED_EVALS,
        std::back_inserter(positions),
        [](const auto& expected) { return chess::notation::from_fen(expected.fen).value(); });

    return positions;
}

void check_trace(const Position& position)
{
    const auto trace = evaluate_traced(position);

    REQUIRE(trace.sideToMove == position.sideToMove);

    REQUIRE(trace.score(DEFAULT_WEIGHTS) == evaluate(position));
}

// checks the trace for every position in the move tree, to the given depth
void check_tree( // NOLINT(misc-no-recursion)
    const Position& position, const size_t depth)
{
    check_trace(position);

    if (depth == 0uz)
        return;

    for (const auto& move : chess::moves::generate(position))
        check_tree(after_move(position, move), depth - 1uz);
}

} // namespace

TEST_CASE("Trace - the eval rebuilt from the trace is exact", "[eval][trace]")
{
    SECTION("Test positions")
    {
        for (const auto& position : get_test_positions()) {
            INFO(chess::notation::to_fen(position));

            check_trace(position);
        }
    }

    SECTION("Move trees")
    {
        for (const auto& position : get_test_positions()) {
            INFO(chess::notation::to_fen(position));

            check_tree(position, 2uz);
        }
    }
}

TEST_CASE("Trace - the traced eval doesn't use the pawn table", "[eval][trace][PawnTable]")
{
    // the cached path gives the same result as the traced one, before & after the table is filled
    for (const auto& position : get_test_positions()) {
        INFO(chess::notation::to_fen(position));

        ben_bot::PawnTable table;

        const auto traced = evaluate_traced(position).score(DEFAULT_WEIGHTS);

        REQUIRE(evaluate(position, &table) == traced); // miss
        REQUIRE(evaluate(position, &table) == traced); // hit

        REQUIRE(evaluate_traced(position).score(DEFAULT_WEIGHTS) == traced);
    }
}

TEST_CASE("Trace - changing one weight changes the eval by its coefficient", "[eval][trace]")
{
    using chess::pieces::Type;

    static constexpr auto NUM_SQUARES = chess::board::NUM_SQUARES;

    // a few terms of different kinds: material, a piece square table entry, an indexed table,
    // a king safety term, pawn structure, mobility, a term that counts our squares minus theirs,
    // and a threat term
    static constexpr std::array indices {
        offset_of(Term::Material) + static_cast<size_t>(Type::Pawn),
        offset_of(Term::Material) + static_cast<size_t>(Type::Rook),
        offset_of(Term::Pst) + (static_cast<size_t>(Type::Knight) * NUM_SQUARES) + 21uz, // f3
        offset_of(Term::PassedPawn) + 2uz,
        offset_of(Term::KnightTropism),
        offset_of(Term::IsolatedPawn),
        offset_of(Term::RookMobility) + 7uz,
        offset_of(Term::Space),
        offset_of(Term::Hanging)
    };

    static constexpr TaperedScore delta { .mg = 7, .eg = -3 };

    // so that the test isn't vacuous, each of these terms must apply in at least one position
    std::array<bool, indices.size()> applied { };

    for (const auto& position : get_test_positions()) {
        INFO(chess::notation::to_fen(position));

        const auto trace = evaluate_traced(position);

        const auto original = trace.white_score(DEFAULT_WEIGHTS);

        for (auto i = 0uz; i < indices.size(); ++i) {
            const auto index = indices.at(i);

            INFO(index);

            auto weights = DEFAULT_WEIGHTS;

            weights.values.at(index) += delta;

            const auto coefficient = trace.coefficient(index);

            REQUIRE(trace.white_score(weights) - original == delta * coefficient);

            if (coefficient != 0)
                applied.at(i) = true;
        }
    }

    for (const auto termApplied : applied)
        REQUIRE(termApplied);
}

TEST_CASE("Trace - coefficients", "[eval][trace]")
{
    using chess::pieces::Type;

    static constexpr auto NUM_SQUARES = chess::board::NUM_SQUARES;

    auto coeffs_for = [](const CoeffTracer& trace, const size_t index, const Color side) {
        return static_cast<int>(trace.coeffs.at(index).at(std::to_underlying(side)));
    };

    SECTION("Starting position")
    {
        const auto trace = evaluate_traced(Position { });

        const auto pawns   = offset_of(Term::Material) + static_cast<size_t>(Type::Pawn);
        const auto knights = offset_of(Term::Material) + static_cast<size_t>(Type::Knight);
        const auto queens  = offset_of(Term::Material) + static_cast<size_t>(Type::Queen);

        REQUIRE(coeffs_for(trace, pawns, Color::White) == 8);
        REQUIRE(coeffs_for(trace, pawns, Color::Black) == 8);
        REQUIRE(coeffs_for(trace, knights, Color::White) == 2);
        REQUIRE(coeffs_for(trace, knights, Color::Black) == 2);
        REQUIRE(coeffs_for(trace, queens, Color::White) == 1);
        REQUIRE(coeffs_for(trace, queens, Color::Black) == 1);

        // the piece square tables are from White's point of view, so the knights on b1 & b8 use the same entry
        const auto b1Knight = offset_of(Term::Pst) + (static_cast<size_t>(Type::Knight) * NUM_SQUARES) + 1uz;

        REQUIRE(coeffs_for(trace, b1Knight, Color::White) == 1);
        REQUIRE(coeffs_for(trace, b1Knight, Color::Black) == 1);

        REQUIRE(trace.phase == ben_bot::eval::MAX_PHASE);
        REQUIRE(trace.base == TaperedScore { });

        for (auto i = 0uz; i < ben_bot::eval::NUM_WEIGHTS; ++i)
            REQUIRE(trace.coefficient(i) == 0);
    }

    SECTION("King & pawn vs king")
    {
        // white has a passed pawn on e2, 6 squares from promoting
        const auto trace = evaluate_traced(chess::notation::from_fen("4k3/8/8/8/8/8/4P3/4K3 b - - 0 1").value());

        REQUIRE(trace.sideToMove == Color::Black);
        REQUIRE(trace.phase == 0);

        REQUIRE(trace.coefficient(offset_of(Term::Material) + static_cast<size_t>(Type::Pawn)) == 1);
        REQUIRE(trace.coefficient(offset_of(Term::PassedPawn) + 5uz) == 1);
        REQUIRE(trace.coefficient(offset_of(Term::IsolatedPawn)) == 1);

        // the white king on e1 is 1 square from the passer
        REQUIRE(coeffs_for(trace, offset_of(Term::KingEscortsPasser), Color::White) == 6);
    }

    SECTION("Game phase")
    {
        auto phase_of = [](const char* fen) {
            return evaluate_traced(chess::notation::from_fen(fen).value()).phase;
        };

        // the queens are traded: 24 - 2 * 4
        REQUIRE(phase_of("rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1") == 16);

        // a rook & a knight each
        REQUIRE(phase_of("4k2r/pppp4/2n5/8/8/2N5/PPPP4/R3K3 w - - 0 1") == 6);

        // each side has promoted to a second queen, which doesn't push the phase past a pure middlegame
        REQUIRE(phase_of("rnbqkbnQ/ppppppp1/8/8/8/8/PPPPPPP1/RNBQKBNq w Qq - 0 1") == ben_bot::eval::MAX_PHASE);
    }

    SECTION("Mop-up & last piece terms go into the base")
    {
        // white is up a queen, so the mop-up & last piece terms apply
        const auto trace = evaluate_traced(chess::notation::from_fen("8/8/8/4k3/8/8/8/KQ6 w - - 0 1").value());

        REQUIRE(trace.base != TaperedScore { });

        for (const auto term : { Term::LastPiece, Term::MopUpEdge, Term::MopUpKingDistance })
            REQUIRE(trace.coefficient(offset_of(term)) == 0);
    }
}
