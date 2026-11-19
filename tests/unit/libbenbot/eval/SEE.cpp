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
#include <libbenbot/eval/SEE.hpp>
#include <libchess/game/Position.hpp>
#include <libchess/notation/FEN.hpp>
#include <libchess/notation/UCI.hpp>
#include <string_view>

inline constexpr auto TAGS { "[eval][SEE]" };

using chess::game::Position;

namespace {

[[nodiscard]] auto see(const std::string_view fen, const std::string_view move) -> int
{
    const auto position = chess::notation::from_fen(fen).value();

    return ben_bot::eval::see(
        position, chess::notation::from_uci(position, move).value());
}

} // namespace

TEST_CASE("SEE - undefended captures", TAGS)
{
    REQUIRE(see("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1", "e4d5") == 100);
    REQUIRE(see("4k3/8/8/3p4/4P3/8/8/4K3 b - - 0 1", "d5e4") == 100);

    // from https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
    REQUIRE(see("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - 0 1", "e1e5") == 100);
}

TEST_CASE("SEE - defended captures", TAGS)
{
    // queen takes a pawn defended by a pawn
    REQUIRE(see("4k3/8/2p5/3p4/8/8/3Q4/4K3 w - - 0 1", "d2d5") == -800);

    // knight takes a knight defended by a pawn
    REQUIRE(see("4k3/8/4p3/3n4/8/4N3/8/4K3 w - - 0 1", "e3d5") == 0);

    // bishop takes a rook defended by a pawn
    REQUIRE(see("4k3/8/2p5/3r4/8/8/6B1/4K3 w - - 0 1", "g2d5") == 170);
}

TEST_CASE("SEE - X-ray attackers", TAGS)
{
    // doubled rooks: without considering the rook on d1, this would lose the exchange
    REQUIRE(see("3rk3/8/8/3p4/8/8/3R4/3R2K1 w - - 0 1", "d2d5") == 100);

    // from https://www.chessprogramming.org/SEE_-_The_Swap_Algorithm
    // both queens are X-ray attackers, behind the rook & bishop
    REQUIRE(see("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1", "d3e5") == -220);
}

TEST_CASE("SEE - king captures", TAGS)
{
    // the king can safely recapture
    REQUIRE(see("4k3/3n4/8/8/8/8/8/3RK3 w - - 0 1", "d1d7") == -180);

    // the king can't recapture, because the square is still defended by the rook behind
    REQUIRE(see("4k3/3n4/8/8/8/8/3R4/3RK3 w - - 0 1", "d2d7") == 320);

    // the king captures an undefended piece
    REQUIRE(see("4k3/8/8/8/8/8/3p4/4K3 w - - 0 1", "e1d2") == 100);
}

TEST_CASE("SEE - quiet moves", TAGS)
{
    REQUIRE(see("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e2e4") == 0);

    // queen moves to a square attacked by a pawn
    REQUIRE(see("4k3/8/8/2p5/8/8/8/3QK3 w - - 0 1", "d1d4") == -900);

    // castling
    REQUIRE(see("4k3/8/8/8/8/8/8/4K2R w K - 0 1", "e1g1") == 0);
}

TEST_CASE("SEE - en passant", TAGS)
{
    REQUIRE(see("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6") == 100);

    // the capturing pawn can be recaptured
    REQUIRE(see("4k3/2p5/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6") == 0);

    // removing the captured pawn uncovers the rook's attack on the target square
    REQUIRE(see("4k3/8/8/3pP3/8/8/7K/3r4 w - d6 0 1", "e5d6") == 0);
}

TEST_CASE("SEE - promotions", TAGS)
{
    REQUIRE(see("4k3/1P6/8/8/8/8/8/4K3 w - - 0 1", "b7b8q") == 800);

    // the promoted queen is captured
    REQUIRE(see("r3k3/1P6/8/8/8/8/8/4K3 w - - 0 1", "b7b8q") == -100);
    REQUIRE(see("r3k3/1P6/8/8/8/8/8/4K3 w - - 0 1", "b7b8n") == -100);

    // capture with promotion
    REQUIRE(see("1r2k3/P7/8/8/8/8/8/4K3 w - - 0 1", "a7b8q") == 1300);

    // the recapture promotes
    REQUIRE(see("1Nr1k3/P7/8/8/8/8/8/4K3 b - - 0 1", "c8b8") == -980);
}
