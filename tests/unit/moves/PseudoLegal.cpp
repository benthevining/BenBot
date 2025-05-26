/*
 * ======================================================================================
 *
 * libchess - a chess engine by Ben Vining
 *
 * ======================================================================================
 */

#include <catch2/catch_test_macros.hpp>
#include <libchess/board/Bitboard.hpp>
#include <libchess/board/Distances.hpp>
#include <libchess/board/File.hpp>
#include <libchess/board/Rank.hpp>
#include <libchess/board/Square.hpp>
#include <libchess/moves/PseudoLegal.hpp>

static constexpr auto TAGS { "[moves][Generation]" };

using chess::board::Bitboard;
using chess::board::File;
using chess::board::Rank;
using chess::board::Square;

namespace move_gen = chess::moves::pseudo_legal;

// pawn pushes
// pawn double pushes
// pawn attacks
// bishop
// rook
// queen

TEST_CASE("Knight move generation", TAGS)
{
    using chess::board::knight_distance;

    SECTION("From D4")
    {
        static constexpr Square starting { File::D, Rank::Four };

        static constexpr auto moves = move_gen::knight(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 8uz);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Five }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::E, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Three }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From A1")
    {
        static constexpr Square starting { File::A, Rank::One };

        static constexpr auto moves = move_gen::knight(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Two }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From A8")
    {
        static constexpr Square starting { File::A, Rank::Eight };

        static constexpr auto moves = move_gen::knight(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Six }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Seven }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From H1")
    {
        static constexpr Square starting { File::H, Rank::One };

        static constexpr auto moves = move_gen::knight(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Three }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From H8")
    {
        static constexpr Square starting { File::H, Rank::Eight };

        static constexpr auto moves = move_gen::knight(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 2u);

        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Six }));

        for (const auto square : moves.squares())
            REQUIRE(knight_distance(starting, square) == 1uz);
    }

    SECTION("From A1 and H1")
    {
        Bitboard starting;

        starting.set({ File::A, Rank::One });
        starting.set({ File::H, Rank::One });

        const auto moves = move_gen::knight(starting);

        REQUIRE(moves.count() == 4uz);

        REQUIRE(moves.test(Square { File::B, Rank::Three }));
        REQUIRE(moves.test(Square { File::C, Rank::Two }));
        REQUIRE(moves.test(Square { File::F, Rank::Two }));
        REQUIRE(moves.test(Square { File::G, Rank::Three }));
    }
}

TEST_CASE("King move generation", TAGS)
{
    SECTION("From G2")
    {
        static constexpr Square starting { File::G, Rank::Two };

        static constexpr auto moves = move_gen::king(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 8u);

        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Three }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::F, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::H, Rank::One }));
    }

    SECTION("From A1")
    {
        static constexpr Square starting { File::A, Rank::One };

        static constexpr auto moves = move_gen::king(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
    }

    SECTION("From A8")
    {
        static constexpr Square starting { File::A, Rank::Eight };

        static constexpr auto moves = move_gen::king(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::A, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Eight }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Seven }));
    }

    SECTION("From H1")
    {
        static constexpr Square starting { File::H, Rank::One };

        static constexpr auto moves = move_gen::king(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::One }));
    }

    SECTION("From H8")
    {
        static constexpr Square starting { File::H, Rank::Eight };

        static constexpr auto moves = move_gen::king(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 3u);

        STATIC_REQUIRE(moves.test(Square { File::H, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        STATIC_REQUIRE(moves.test(Square { File::G, Rank::Eight }));
    }

    SECTION("From C1")
    {
        static constexpr Square starting { File::C, Rank::One };

        static constexpr auto moves = move_gen::king(Bitboard { starting });

        STATIC_REQUIRE(moves.count() == 5u);

        STATIC_REQUIRE(moves.test(Square { File::B, Rank::One }));
        STATIC_REQUIRE(moves.test(Square { File::B, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::C, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::Two }));
        STATIC_REQUIRE(moves.test(Square { File::D, Rank::One }));
    }

    SECTION("From A1 and H8")
    {
        Bitboard starting;

        starting.set({ File::A, Rank::One });
        starting.set({ File::H, Rank::Eight });

        const auto moves = move_gen::king(starting);

        REQUIRE(moves.count() == 6uz);

        REQUIRE(moves.test(Square { File::A, Rank::Two }));
        REQUIRE(moves.test(Square { File::B, Rank::One }));
        REQUIRE(moves.test(Square { File::B, Rank::Two }));
        REQUIRE(moves.test(Square { File::H, Rank::Seven }));
        REQUIRE(moves.test(Square { File::G, Rank::Seven }));
        REQUIRE(moves.test(Square { File::G, Rank::Eight }));
    }
}
