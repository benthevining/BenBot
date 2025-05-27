/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/BitboardMasks.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/PseudoLegal.hpp>
#include <libchess/pieces/Colors.hpp>

static constexpr auto TAGS { "[moves][Generation][pseudo-legal]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;
using chess::pieces::Color;

namespace board_masks = chess::board::masks;
namespace move_gen    = chess::moves::pseudo_legal;

TEST_CASE("Pseudo-legal - pawn pushes", TAGS)
{
    SECTION("White")
    {
        SECTION("From starting position")
        {
            static constexpr auto startingPos = board_masks::starting::white::pawns();

            static constexpr auto allPushes = move_gen::pawn_pushes(
                startingPos, Color::White, {});

            STATIC_REQUIRE(allPushes == board_masks::ranks::three());

            static constexpr auto pushes = move_gen::pawn_pushes(
                startingPos, Color::White, Bitboard { Square { File::A, Rank::Three } });

            STATIC_REQUIRE(pushes.count() == 7uz);

            STATIC_REQUIRE(! pushes.test(Square { File::A, Rank::Three }));
        }

        SECTION("From D7")
        {
            static constexpr Bitboard starting { Square { File::D, Rank::Seven } };

            static constexpr auto allPushes = move_gen::pawn_pushes(
                starting, Color::White, {});

            STATIC_REQUIRE(allPushes.count() == 1uz);
            STATIC_REQUIRE(allPushes.test(Square { File::D, Rank::Eight }));

            static constexpr auto pushes = move_gen::pawn_pushes(
                starting, Color::White, Bitboard { Square { File::D, Rank::Eight } });

            STATIC_REQUIRE(pushes.none());
        }
    }

    SECTION("Black")
    {
        SECTION("From starting position")
        {
            static constexpr auto startingPos = board_masks::starting::black::pawns();

            static constexpr auto allPushes = move_gen::pawn_pushes(
                startingPos, Color::Black, {});

            STATIC_REQUIRE(allPushes == board_masks::ranks::six());

            static constexpr auto pushes = move_gen::pawn_pushes(
                startingPos, Color::Black, Bitboard { Square { File::C, Rank::Six } });

            STATIC_REQUIRE(pushes.count() == 7uz);

            STATIC_REQUIRE(! pushes.test(Square { File::C, Rank::Six }));
        }

        SECTION("From E2")
        {
            static constexpr Bitboard starting { Square { File::E, Rank::Two } };

            static constexpr auto allPushes = move_gen::pawn_pushes(
                starting, Color::Black, {});

            STATIC_REQUIRE(allPushes.count() == 1uz);
            STATIC_REQUIRE(allPushes.test(Square { File::E, Rank::One }));

            static constexpr auto pushes = move_gen::pawn_pushes(
                starting, Color::Black, Bitboard { Square { File::E, Rank::One } });

            STATIC_REQUIRE(pushes.none());
        }
    }
}

TEST_CASE("Pseudo-legal - pawn double pushes", TAGS)
{
    SECTION("White")
    {
        static constexpr auto starting = board_masks::starting::white::pawns();

        static constexpr auto allPushes = move_gen::pawn_double_pushes(starting, Color::White, {});

        STATIC_REQUIRE(allPushes == board_masks::ranks::four());

        Bitboard occupied;

        occupied.set(Square { File::A, Rank::Three });
        occupied.set(Square { File::F, Rank::Three });

        const auto pushes = move_gen::pawn_double_pushes(starting, Color::White, occupied);

        REQUIRE(pushes.count() == 6uz);

        REQUIRE(pushes.test(Square { File::B, Rank::Four }));
        REQUIRE(pushes.test(Square { File::C, Rank::Four }));
        REQUIRE(pushes.test(Square { File::D, Rank::Four }));
        REQUIRE(pushes.test(Square { File::E, Rank::Four }));
        REQUIRE(pushes.test(Square { File::G, Rank::Four }));
        REQUIRE(pushes.test(Square { File::H, Rank::Four }));
    }

    SECTION("Black")
    {
        static constexpr auto starting = board_masks::starting::black::pawns();

        static constexpr auto allPushes = move_gen::pawn_double_pushes(starting, Color::Black, {});

        STATIC_REQUIRE(allPushes == board_masks::ranks::five());

        Bitboard occupied;

        occupied.set(Square { File::B, Rank::Six });
        occupied.set(Square { File::D, Rank::Six });
        occupied.set(Square { File::H, Rank::Six });

        const auto pushes = move_gen::pawn_double_pushes(starting, Color::Black, occupied);

        REQUIRE(pushes.count() == 5uz);

        REQUIRE(pushes.test(Square { File::A, Rank::Five }));
        REQUIRE(pushes.test(Square { File::C, Rank::Five }));
        REQUIRE(pushes.test(Square { File::E, Rank::Five }));
        REQUIRE(pushes.test(Square { File::F, Rank::Five }));
        REQUIRE(pushes.test(Square { File::G, Rank::Five }));
    }
}

TEST_CASE("Pseudo-legal - pawn captures", TAGS)
{
    SECTION("White")
    {
        SECTION("D4 -> C5")
        {
            static constexpr Bitboard startingPawns { Square { File::D, Rank::Four } };

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::A, Rank::Two });
            enemyPieces.set(Square { File::B, Rank::Eight });
            enemyPieces.set(Square { File::C, Rank::Five });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::White, enemyPieces);

            REQUIRE(captures.count() == 1uz);

            REQUIRE(captures.test(Square { File::C, Rank::Five }));
        }

        SECTION("From C2, none available")
        {
            static constexpr Bitboard startingPawns { Square { File::C, Rank::Two } };

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::A, Rank::Seven });
            enemyPieces.set(Square { File::F, Rank::Four });
            enemyPieces.set(Square { File::G, Rank::Six });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::White, enemyPieces);

            REQUIRE(captures.none());
        }

        SECTION("F7 -> E8/G8")
        {
            static constexpr Bitboard startingPawns { Square { File::F, Rank::Seven } };

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::E, Rank::Eight });
            enemyPieces.set(Square { File::G, Rank::Eight });
            enemyPieces.set(Square { File::C, Rank::Two });
            enemyPieces.set(Square { File::F, Rank::Six });
            enemyPieces.set(Square { File::E, Rank::Seven });
            enemyPieces.set(Square { File::A, Rank::Four });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::White, enemyPieces);

            REQUIRE(captures.count() == 2uz);

            REQUIRE(captures.test(Square { File::E, Rank::Eight }));
            REQUIRE(captures.test(Square { File::G, Rank::Eight }));
        }

        SECTION("E4 -> D5/F5 and F6 -> G7")
        {
            Bitboard startingPawns;

            startingPawns.set(Square { File::E, Rank::Four });
            startingPawns.set(Square { File::F, Rank::Six });

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::D, Rank::Five });
            enemyPieces.set(Square { File::F, Rank::Five });
            enemyPieces.set(Square { File::G, Rank::Seven });
            enemyPieces.set(Square { File::A, Rank::Two });
            enemyPieces.set(Square { File::B, Rank::Eight });
            enemyPieces.set(Square { File::H, Rank::Three });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::White, enemyPieces);

            REQUIRE(captures.count() == 3uz);

            REQUIRE(captures.test(Square { File::D, Rank::Five }));
            REQUIRE(captures.test(Square { File::F, Rank::Five }));
            REQUIRE(captures.test(Square { File::G, Rank::Seven }));
        }
    }

    SECTION("Black")
    {
        SECTION("E5 -> D4")
        {
            static constexpr Bitboard startingPawns { Square { File::E, Rank::Five } };

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::D, Rank::Four });
            enemyPieces.set(Square { File::F, Rank::Six });
            enemyPieces.set(Square { File::G, Rank::One });
            enemyPieces.set(Square { File::A, Rank::Seven });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::Black, enemyPieces);

            REQUIRE(captures.count() == 1uz);

            REQUIRE(captures.test(Square { File::D, Rank::Four }));
        }

        SECTION("From F6, none available")
        {
            static constexpr Bitboard startingPawns { Square { File::F, Rank::Six } };

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::G, Rank::Seven });
            enemyPieces.set(Square { File::E, Rank::Seven });
            enemyPieces.set(Square { File::A, Rank::Two });
            enemyPieces.set(Square { File::H, Rank::Eight });
            enemyPieces.set(Square { File::C, Rank::One });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::Black, enemyPieces);

            REQUIRE(captures.none());
        }

        SECTION("G2 -> F1/H1")
        {
            static constexpr Bitboard startingPawns { Square { File::G, Rank::Two } };

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::F, Rank::One });
            enemyPieces.set(Square { File::H, Rank::One });
            enemyPieces.set(Square { File::A, Rank::Six });
            enemyPieces.set(Square { File::G, Rank::One });
            enemyPieces.set(Square { File::H, Rank::One });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::Black, enemyPieces);

            REQUIRE(captures.count() == 2uz);

            REQUIRE(captures.test(Square { File::F, Rank::One }));
            REQUIRE(captures.test(Square { File::H, Rank::One }));
        }

        SECTION("F4 -> E3/G3 and B3 -> A2")
        {
            Bitboard startingPawns;

            startingPawns.set(Square { File::F, Rank::Four });
            startingPawns.set(Square { File::B, Rank::Three });

            Bitboard enemyPieces;

            enemyPieces.set(Square { File::E, Rank::Three });
            enemyPieces.set(Square { File::G, Rank::Three });
            enemyPieces.set(Square { File::A, Rank::Two });
            enemyPieces.set(Square { File::F, Rank::Eight });
            enemyPieces.set(Square { File::E, Rank::Six });
            enemyPieces.set(Square { File::A, Rank::Eight });

            const auto captures = move_gen::pawn_captures(startingPawns, Color::Black, enemyPieces);

            REQUIRE(captures.count() == 3uz);

            REQUIRE(captures.test(Square { File::E, Rank::Three }));
            REQUIRE(captures.test(Square { File::G, Rank::Three }));
            REQUIRE(captures.test(Square { File::A, Rank::Two }));
        }
    }
}

TEST_CASE("Pseudo-legal - knights", TAGS)
{
    SECTION("From D1")
    {
        static constexpr Bitboard starting { Square { File::D, Rank::One } };

        static constexpr auto allMoves = move_gen::knight(starting, {});

        STATIC_REQUIRE(allMoves.count() == 4uz);

        STATIC_REQUIRE(allMoves.test(Square { File::B, Rank::Two }));
        STATIC_REQUIRE(allMoves.test(Square { File::C, Rank::Three }));
        STATIC_REQUIRE(allMoves.test(Square { File::E, Rank::Three }));
        STATIC_REQUIRE(allMoves.test(Square { File::F, Rank::Two }));

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::C, Rank::Three });
        friendlyPieces.set(Square { File::F, Rank::Two });

        const auto moves = move_gen::knight(starting, friendlyPieces);

        REQUIRE(moves.count() == 2uz);

        REQUIRE(moves.test(Square { File::B, Rank::Two }));
        REQUIRE(moves.test(Square { File::E, Rank::Three }));
    }

    SECTION("From E4 and D4")
    {
        Bitboard starting;

        starting.set(Square { File::E, Rank::Four });
        starting.set(Square { File::D, Rank::Four });

        const auto allMoves = move_gen::knight(starting, {});

        REQUIRE(allMoves.count() == 16uz);

        REQUIRE(allMoves == Bitboard { 0x3C6600663C00 });

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::E, Rank::Four });
        friendlyPieces.set(Square { File::D, Rank::Four });
        friendlyPieces.set(Square { File::B, Rank::Three });
        friendlyPieces.set(Square { File::G, Rank::Five });
        friendlyPieces.set(Square { File::D, Rank::Six });
        friendlyPieces.set(Square { File::F, Rank::Two });

        const auto moves = move_gen::knight(starting, friendlyPieces);

        REQUIRE(moves.count() == 12uz);

        REQUIRE(moves == Bitboard { 0x342600641C00 });
    }
}

TEST_CASE("Pseudo-legal - bishops", TAGS)
{
    SECTION("From D4")
    {
        static constexpr Square starting { File::D, Rank::Four };

        static constexpr Bitboard enemyPieces { Square { File::G, Rank::Seven } };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::B, Rank::Two });
        friendlyPieces.set(Square { File::G, Rank::One });

        const auto moves = move_gen::bishop(
            starting, friendlyPieces | enemyPieces, friendlyPieces);

        REQUIRE(moves.count() == 9uz);

        REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        REQUIRE(! moves.test(Square { File::B, Rank::Two }));
        REQUIRE(! moves.test(Square { File::G, Rank::One }));

        REQUIRE(moves == Bitboard { 0X41221400142000 });
    }

    SECTION("From G6")
    {
        static constexpr Square starting { File::G, Rank::Six };

        static constexpr Bitboard friendlyPieces { Square { File::B, Rank::One } };

        Bitboard enemyPieces;

        enemyPieces.set(Square { File::C, Rank::Two });
        enemyPieces.set(Square { File::D, Rank::Three });

        const auto moves = move_gen::bishop(
            starting, friendlyPieces | enemyPieces, friendlyPieces);

        REQUIRE(moves.count() == 7uz);

        REQUIRE(moves.test(Square { File::D, Rank::Three }));
        REQUIRE(! moves.test(Square { File::C, Rank::Two }));
        REQUIRE(! moves.test(Square { File::B, Rank::One }));

        REQUIRE(moves == Bitboard { 0X10A000A010080000 });
    }
}

TEST_CASE("Pseudo-legal - rooks", TAGS)
{
    SECTION("From C3")
    {
        static constexpr Square starting { File::C, Rank::Three };

        static constexpr Bitboard friendlyPieces { Square { File::B, Rank::Three } };

        Bitboard enemyPieces;

        enemyPieces.set(Square { File::F, Rank::Three });
        enemyPieces.set(Square { File::G, Rank::Three });
        enemyPieces.set(Square { File::H, Rank::Three });
        enemyPieces.set(Square { File::C, Rank::Eight });
        enemyPieces.set(Square { File::C, Rank::Six });

        const auto moves = move_gen::rook(
            starting, friendlyPieces | enemyPieces, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X40404380404 });
    }

    SECTION("From E7")
    {
        static constexpr Square starting { File::E, Rank::Seven };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::B, Rank::Seven });
        friendlyPieces.set(Square { File::E, Rank::Four });

        static constexpr Bitboard enemyPieces { Square { File::E, Rank::Eight } };

        const auto moves = move_gen::rook(
            starting, friendlyPieces | enemyPieces, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X10EC101000000000 });
    }
}

TEST_CASE("Pseudo-legal - queens", TAGS)
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

        const auto moves = move_gen::queen(
            starting, friendlyPieces | enemyPieces, friendlyPieces);

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

        const auto moves = move_gen::queen(
            starting, friendlyPieces | enemyPieces, friendlyPieces);

        REQUIRE(moves == Bitboard { 0X20120A071D });
    }
}

TEST_CASE("Pseudo-legal - kings", TAGS)
{
    SECTION("From G4")
    {
        static constexpr Bitboard starting { Square { File::G, Rank::Four } };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::H, Rank::Five });
        friendlyPieces.set(Square { File::G, Rank::Three });

        const auto moves = move_gen::king(starting, friendlyPieces);

        REQUIRE(moves.count() == 6uz);

        REQUIRE(moves.test(Square { File::F, Rank::Five }));
        REQUIRE(moves.test(Square { File::G, Rank::Five }));
        REQUIRE(moves.test(Square { File::F, Rank::Four }));
        REQUIRE(moves.test(Square { File::H, Rank::Four }));
        REQUIRE(moves.test(Square { File::F, Rank::Three }));
        REQUIRE(moves.test(Square { File::H, Rank::Three }));
    }

    SECTION("From A8")
    {
        static constexpr Bitboard starting { Square { File::A, Rank::Eight } };

        Bitboard friendlyPieces;

        friendlyPieces.set(Square { File::A, Rank::Seven });
        friendlyPieces.set(Square { File::B, Rank::Seven });
        friendlyPieces.set(Square { File::C, Rank::Seven });

        const auto moves = move_gen::king(starting, friendlyPieces);

        REQUIRE(moves.count() == 1uz);

        REQUIRE(moves.test(Square { File::B, Rank::Eight }));
    }
}
