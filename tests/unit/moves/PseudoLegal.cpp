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

// pawn double pushes
// pawn captures
// knights
// bishops
// rooks
// queens
// kings

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
