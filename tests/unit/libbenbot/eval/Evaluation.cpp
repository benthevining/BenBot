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
#include <cstddef>
#include <libbenbot/data-structures/PawnTable.hpp>
#include <libbenbot/eval/Evaluation.hpp>
#include <libbenbot/eval/PieceSquareTables.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/moves/MoveGen.hpp>
#include <libchess/notation/FEN.hpp>
#include <string>
#include <string_view>

inline constexpr auto TAGS { "[eval][evaluation][!mayfail]" };

using ben_bot::eval::evaluate;
using chess::game::Position;
using chess::notation::from_fen;

TEST_CASE("Evaluation symmetry", TAGS)
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    SECTION("Starting position")
    {
        static const Position startPos;

        REQUIRE(
            evaluate(startPos, pieceSquareTables)
            == evaluate(flipped(startPos), pieceSquareTables));
    }

    SECTION("Kiwipete")
    {
        const auto position = from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Mate in 1")
    {
        const auto position = from_fen("r2qkb1r/ppp1p1pp/2n3p1/1b6/2BpP3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Mate")
    {
        const auto position = from_fen("8/8/8/8/8/8/3q4/2K1k3 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Promoting")
    {
        const auto position = from_fen("8/2P5/8/8/k1K5/8/8/8 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Uneven piece mobility")
    {
        const auto position = from_fen("r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Pawn attacking a rook")
    {
        const auto position = from_fen("4k3/8/2r5/3P4/8/8/8/4K3 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Pawn forking a knight & bishop")
    {
        const auto position = from_fen("r2qk2r/ppp2ppp/3n1b2/4P3/8/8/PPP2PPP/R2QK2R b KQkq - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Knight forking a rook & queen")
    {
        const auto position = from_fen("2r1k3/3q4/1N6/8/8/8/8/4K3 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Bishop attacking a rook")
    {
        const auto position = from_fen("4k3/8/4r3/8/2B5/8/8/4K3 b - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Rook attacking a queen")
    {
        const auto position = from_fen("6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Rook attacked by a pawn & a knight")
    {
        const auto position = from_fen("6k1/8/8/4r3/3P4/5N2/8/6K1 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Hanging bishop - white to move")
    {
        const auto position = from_fen("6k1/7p/4b3/8/8/8/4R3/6K1 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Hanging bishop - black to move")
    {
        const auto position = from_fen("6k1/7p/4b3/8/8/8/4R3/6K1 b - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Knight attacked twice & defended once")
    {
        const auto position = from_fen("8/8/4k3/4n3/2N5/4NN2/6K1/8 b - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }

    SECTION("Hanging pieces on both sides")
    {
        // the knight on e5 & the bishop on d3 attack each other, and neither is defended
        const auto position = from_fen("6k1/8/8/4n3/8/2BB4/8/6K1 w - - 0 1").value();

        REQUIRE(
            evaluate(position, pieceSquareTables)
            == evaluate(flipped(position), pieceSquareTables));
    }
}

TEST_CASE("Evaluation - mobility", "[eval][evaluation][mobility]")
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    SECTION("Bishop blocked in by its own pawns")
    {
        // The only difference is the white bishop on c1 vs f1, which are mirrored squares, so
        // all the other eval terms are the same. The bishop on c1 can't move.
        const auto trapped = from_fen("2b4k/1p1p4/8/8/8/8/1P1P4/K1B5 w - - 0 1").value();
        const auto free    = from_fen("2b4k/1p1p4/8/8/8/8/1P1P4/K4B2 w - - 0 1").value();

        REQUIRE(evaluate(trapped, pieceSquareTables) < evaluate(free, pieceSquareTables));
    }
}

namespace {

// walks the move tree to the given depth, checking that evaluating with the pawn table
// gives the same result as evaluating without it. The table is shared between all the
// positions visited, so most of the evaluations after the first few are cache hits for
// positions with the same pawns but different pieces
void check_pawn_table_evals( // NOLINT(misc-no-recursion)
    const Position& pos, const size_t depth,
    const ben_bot::eval::PieceSquareTables& pst, ben_bot::PawnTable& table)
{
    REQUIRE(evaluate(pos, pst, &table) == evaluate(pos, pst));

    if (depth == 0uz)
        return;

    for (const auto& move : chess::moves::generate(pos))
        check_pawn_table_evals(after_move(pos, move), depth - 1uz, pst, table);
}

} // namespace

TEST_CASE("Evaluation - pawn table gives the same result", "[eval][evaluation][PawnTable]")
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    SECTION("Miss, then hit")
    {
        static constexpr std::array<std::string_view, 8> fens {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "8/2P5/8/8/k1K5/8/8/8 w - - 0 1",
            "8/8/8/8/8/8/3q4/2K1k3 w - - 0 1",         // no pawns
            "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1", // rook behind a passer
            "r1b2rk1/pp1nqppp/2p1p3/3n4/2BP4/2N1PN2/PPQ2PPP/R4RK1 w - - 0 1"
        };

        for (const auto fen : fens) {
            const auto pos = from_fen(fen).value();

            ben_bot::PawnTable table;

            const auto uncached = evaluate(pos, pieceSquareTables);

            // empty entries must never look like a hit, even with no pawns on the board
            REQUIRE(table.probe(pos.pawnHash).key != pos.pawnHash);

            REQUIRE(evaluate(pos, pieceSquareTables, &table) == uncached); // miss

            REQUIRE(table.probe(pos.pawnHash).key == pos.pawnHash);

            REQUIRE(evaluate(pos, pieceSquareTables, &table) == uncached); // hit

            // the same position with the other side to move has the same pawn hash
            const auto nullMove = after_null_move(pos);

            REQUIRE(evaluate(nullMove, pieceSquareTables, &table) == evaluate(nullMove, pieceSquareTables));
        }
    }

    SECTION("Same pawns, different pieces")
    {
        // the passed pawns' rook & king terms aren't cached, so moving the rooks & kings
        // around these passers must still change the eval
        static constexpr std::array<std::string_view, 6> fens {
            "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1",
            "1r4k1/5ppp/8/1P6/8/8/5PPP/6K1 w - - 0 1",
            "6k1/5ppp/8/1P6/8/8/5PPP/6KR w - - 0 1",
            "1k6/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1",
            "6k1/5ppp/8/1P6/K7/8/5PPP/1R6 w - - 0 1",
            "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 b - - 0 1"
        };

        ben_bot::PawnTable table;

        for (const auto fen : fens) {
            const auto pos = from_fen(fen).value();

            REQUIRE(evaluate(pos, pieceSquareTables, &table) == evaluate(pos, pieceSquareTables));
        }
    }

    SECTION("Move tree walks")
    {
        for (const auto fen : {
                 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                 "6k1/5ppp/8/1P6/8/8/5PPP/1R4K1 w - - 0 1" }) {
            ben_bot::PawnTable table;

            check_pawn_table_evals(from_fen(fen).value(), 2uz, pieceSquareTables, table);
        }
    }
}

TEST_CASE("Evaluation - threats", "[eval][evaluation][threats]")
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    // The black knight is on e5 (attacked by the white pawn) or d5 (not attacked). These are
    // mirrored squares, so the knight's placement & mobility score the same.

    SECTION("Pawn attacking a knight - white to move")
    {
        const auto attacked = from_fen("6k1/8/8/4n3/3P4/8/8/6K1 w - - 0 1").value();
        const auto safe     = from_fen("6k1/8/8/3n4/3P4/8/8/6K1 w - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) > evaluate(safe, pieceSquareTables));
    }

    SECTION("Pawn attacking a knight - black to move")
    {
        const auto attacked = from_fen("6k1/8/8/4n3/3P4/8/8/6K1 b - - 0 1").value();
        const auto safe     = from_fen("6k1/8/8/3n4/3P4/8/8/6K1 b - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) < evaluate(safe, pieceSquareTables));
    }

    // In the rest of these, the black piece is on e6 (attacked) or d6 (not attacked), which are
    // also mirrored squares.

    SECTION("Knight attacking a rook")
    {
        const auto attacked = from_fen("6k1/8/4r3/8/5N2/8/8/6K1 w - - 0 1").value();
        const auto safe     = from_fen("6k1/8/3r4/8/5N2/8/8/6K1 w - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) > evaluate(safe, pieceSquareTables));
    }

    SECTION("Bishop attacking a rook")
    {
        const auto attacked = from_fen("6k1/8/4r3/8/2B5/8/8/6K1 b - - 0 1").value();
        const auto safe     = from_fen("6k1/8/3r4/8/2B5/8/8/6K1 b - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) < evaluate(safe, pieceSquareTables));
    }

    SECTION("Knight attacking a queen")
    {
        const auto attacked = from_fen("6k1/8/4q3/6N1/8/8/PPP5/1K6 w - - 0 1").value();
        const auto safe     = from_fen("6k1/8/3q4/6N1/8/8/PPP5/1K6 w - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) > evaluate(safe, pieceSquareTables));
    }

    SECTION("Rook attacking a queen")
    {
        const auto attacked = from_fen("6k1/8/4q3/8/8/8/4R3/6K1 w - - 0 1").value();
        const auto safe     = from_fen("6k1/8/3q4/8/8/8/4R3/6K1 w - - 0 1").value();

        REQUIRE(evaluate(attacked, pieceSquareTables) > evaluate(safe, pieceSquareTables));
    }
}

TEST_CASE("Evaluation - hanging pieces", "[eval][evaluation][threats]")
{
    const auto pieceSquareTables = ben_bot::eval::PieceSquareTables::get_default();

    // In each pair, the black knight is on d5 or e5, which are mirrored squares, and the white
    // knights on e3 & f3 attack d5 & e5 once each. The pairs differ in how the black knight is
    // attacked & defended. Each pair is checked with white to move, with black to move, and with
    // the colours swapped (so that black is the attacker).

    // hangingFEN & safeFEN have white to move
    auto check_pair = [&pieceSquareTables](const std::string_view hangingFEN, const std::string_view safeFEN) {
        const auto hanging = from_fen(hangingFEN).value();
        const auto safe    = from_fen(safeFEN).value();

        REQUIRE(evaluate(hanging, pieceSquareTables) > evaluate(safe, pieceSquareTables));

        REQUIRE(evaluate(flipped(hanging), pieceSquareTables) > evaluate(flipped(safe), pieceSquareTables));

        auto black_to_move = [](const std::string_view fen) {
            auto str = std::string { fen };
            str.replace(str.find(" w "), 3, " b ");
            return from_fen(str).value();
        };

        REQUIRE(
            evaluate(black_to_move(hangingFEN), pieceSquareTables)
            < evaluate(black_to_move(safeFEN), pieceSquareTables));
    };

    SECTION("Undefended knight vs knight defended by a pawn")
    {
        // the pawn on d6 defends e5, but not d5
        check_pair(
            "7k/8/3p4/3n4/8/4NN2/5K2/8 w - - 0 1",
            "7k/8/3p4/4n3/8/4NN2/5K2/8 w - - 0 1");
    }

    SECTION("Attacked once & undefended vs attacked once & defended once")
    {
        // the knight on c6 defends e5, but not d5. Attacked once & defended once isn't hanging.
        check_pair(
            "7k/8/2n5/3n4/8/4NN2/5K2/8 w - - 0 1",
            "7k/8/2n5/4n3/8/4NN2/5K2/8 w - - 0 1");
    }

    SECTION("Attacked twice & defended once vs attacked once & defended once")
    {
        // the black king defends both d5 & e5. The knights on c4 & f3 both attack e5,
        // but only the one on e3 attacks d5.
        check_pair(
            "8/8/4k3/4n3/2N5/4NN2/6K1/8 w - - 0 1",
            "8/8/4k3/3n4/2N5/4NN2/6K1/8 w - - 0 1");
    }
}
