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
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/Magics.hpp>

static constexpr auto TAGS { "[moves][Generation][magics]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

namespace move_gen = chess::moves::magics;

TEST_CASE("Magics - bishops", TAGS)
{
    SECTION("From D4")
    {
        static constexpr Square starting { File::D, Rank::Four };

        static constexpr auto enemyPieces = Bitboard::from_square(Square { File::G, Rank::Seven });

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::B, Rank::Two });
        friendlyPieces.set(Square { File::G, Rank::One });

        const auto occupied = friendlyPieces | enemyPieces;

        const auto moves = move_gen::bishop(
            starting, occupied, friendlyPieces);

        REQUIRE(moves.count() == 9uz);

        REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        REQUIRE(! moves.test(Square { File::B, Rank::Two }));
        REQUIRE(! moves.test(Square { File::G, Rank::One }));

        REQUIRE(moves == Bitboard { 0X41221400142000 });
    }

    SECTION("From G6")
    {
        static constexpr Square starting { File::G, Rank::Six };

        static constexpr auto friendlyPieces = Bitboard::from_square(Square { File::B, Rank::One });

        Bitboard enemyPieces;

        enemyPieces.set(Square { File::C, Rank::Two });
        enemyPieces.set(Square { File::D, Rank::Three });

        const auto occupied = friendlyPieces | enemyPieces;

        const auto moves = move_gen::bishop(
            starting, occupied, friendlyPieces);

        REQUIRE(moves.count() == 7uz);

        REQUIRE(moves.test(Square { File::D, Rank::Three }));
        REQUIRE(! moves.test(Square { File::C, Rank::Two }));
        REQUIRE(! moves.test(Square { File::B, Rank::One }));

        REQUIRE(moves == Bitboard { 0X10A000A010080000 });
    }
}

TEST_CASE("Magics - rooks", TAGS)
{
    SECTION("From C3")
    {
        static constexpr Square starting { File::C, Rank::Three };

        static constexpr auto friendlyPieces = Bitboard::from_square(Square { File::B, Rank::Three });

        Bitboard enemyPieces;

        enemyPieces.set(Square { File::F, Rank::Three });
        enemyPieces.set(Square { File::G, Rank::Three });
        enemyPieces.set(Square { File::H, Rank::Three });
        enemyPieces.set(Square { File::C, Rank::Eight });
        enemyPieces.set(Square { File::C, Rank::Six });

        const auto occupied = friendlyPieces | enemyPieces;

        const auto moves = move_gen::rook(
            starting, occupied, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X40404380404 });
    }

    SECTION("From E7")
    {
        static constexpr Square starting { File::E, Rank::Seven };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::B, Rank::Seven });
        friendlyPieces.set(Square { File::E, Rank::Four });

        static constexpr auto enemyPieces = Bitboard::from_square(Square { File::E, Rank::Eight });

        const auto occupied = friendlyPieces | enemyPieces;

        const auto moves = move_gen::rook(
            starting, occupied, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X10EC101000000000 });
    }
}

TEST_CASE("Magics - queens", TAGS)
{
    SECTION("From E5")
    {
        static constexpr Square starting { File::E, Rank::Five };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::B, Rank::Eight });
        friendlyPieces.set(Square { File::F, Rank::Six });
        friendlyPieces.set(Square { File::A, Rank::One });

        Bitboard enemyPieces;

        enemyPieces.set(Square { File::C, Rank::Five });
        enemyPieces.set(Square { File::E, Rank::Three });

        const auto occupied = friendlyPieces | enemyPieces;

        const auto moves = move_gen::queen(
            starting, occupied, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X101418EC38548200 });
    }

    SECTION("From B1")
    {
        static constexpr Square starting { File::B, Rank::One };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::F, Rank::One });
        friendlyPieces.set(Square { File::G, Rank::Six });

        Bitboard enemyPieces;

        enemyPieces.set(Square { File::A, Rank::One });
        enemyPieces.set(Square { File::A, Rank::Two });
        enemyPieces.set(Square { File::B, Rank::Four });

        const auto occupied = friendlyPieces | enemyPieces;

        const auto moves = move_gen::queen(
            starting, occupied, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X20120A071D });
    }
}
